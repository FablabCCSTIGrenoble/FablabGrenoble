<?php

/**
 * A Versioncontrol authorization plugin which builds on a per-repository
 * "account" model.
 *
 * This plugin builds an association between a VersioncontrolRepository and
 * individual Drupal user accounts. It allows for highly granular per-branch and
 * per-tag controls, but also provides easy higher-level permissions. There is
 * no UI, but CRUD is provided, so you should be able to build UIs on top of
 * this plugin, then use its CRUD features to store the data.
 *
 * The plugin maintains a per-user list of repository permissions. These
 * permissions are all stored as an associative array, keyed on the uid to which
 * they belong, in the $userData property. They are as follows:
 *  - 'access': the outermost permission, indicating general access to the
 *    repository.
 *  - 'branch_create': permission indicating whether the user has access to
 *    create branches.
 *  - 'tag_create': permission indicating whether the user has access to create
 *    tags.
 *  - 'branch_update': permission indicating whether the user has access to
 *    update/write to branches.
 *  - 'tag_update': permission indicating whether the user has access to update
 *    or write to tags.
 *  - 'branch_delete': permission indicating whether the user has access to
 *    delete branches.
 *  - 'tag_delete': permission indicating whether the user has access to delete
 *    tags.
 *
 * The system also maintains distinct update & delete permissions on a per-label
 * (branch or tag) basis. For the most part, the above list of permissions are
 * directly recorded in the $userData array using a simple boolean to indicate
 * grant or deny (represented by VersioncontrolAuthHandlerMappedAccounts::DENY
 * and VersioncontrolAuthHandlerMappedAccounts::GRANT). However, to allow for
 * simpler UIs and decrease data synchronization overhead, the plugin uses a
 * cascading auth logic that involves a third possible permission value,
 * VersioncontrolAuthHandlerMappedAccounts::ALL, to be set on some of the above
 * permissions. Here is the effect, in each case:
 *
 *  - 'access': if set to the ALL permission, this grants global authorization
 *    to all operations on the repository, superceding any other DENYs.
 *  - 'branch_update': if set to the ALL permission, authorization to
 *    write to all branches is granted, superceding any any per-branch DENYs.
 *  - 'branch_delete': same principle as 'branch_update', but for branch
 *    deletion: supercedes any per-branch DENYs.
 *  - 'tag_update': same principle as 'branch_update', but for tags.
 *  - 'tag_delete': same principle as 'branch_delete', but for tags.
 *
 * This cascading logical flow can be represented hierarchically, where a parent
 * is capable of superceding its children and producing a definitive auth
 * response:
 *
 *    - 'access'
 *      - 'branch_create'
 *      - 'branch_update'
 *        - (some branch foo) 'update'
 *        - (some branch bar) 'update'
 *      - 'branch_delete'
 *        - (some branch foo) 'delete'
 *        - (some branch bar) 'delete'
 *      - 'tag_create'
 *      - 'tag_update'
 *        - (some tag foo) 'update'
 *        - (some tag bar) 'update'
 *      - 'tag_delete'
 *        - (some tag foo) 'delete'
 *        - (some tag bar) 'delete'
 */
class VersioncontrolAuthHandlerMappedAccounts implements VersioncontrolAuthHandlerInterface {
  /**
   * The repository this plugin is working with.
   *
   * @var VersioncontrolRepository
   */
  protected $repository;

  /**
   * Array containing all the user permissions data for the attached repository.
   *
   * @var array
   */
  protected $userData = array();

  /**
   * Boolean indicating whether the object has already run its build routine.
   *
   * @var bool
   */
  protected $built = FALSE;

  /**
   * An array of error message strings, to be formatted by sprintf when
   * VersioncontrolAuthHandlerMappedAccounts::getErrorMessages is called.
   *
   * @var array
   */
  protected $errors = array();

  /**
   * Permission value indicating access should be denied.
   */
  const DENY  = 0;

  /**
   * Permission value indicating access should be granted.
   *
   */
  const GRANT = 1;

  /**
   * Permission value indicating access should be granted for this perm, AND
   * for all child perms.
   */
  const ALL   = 2;

  public function setRepository(VersioncontrolRepository $repository) {
    if ($this->repository instanceof VersioncontrolRepository && $this->repository !== $repository) {
      throw new Exception('Cannot attach different repositories to a single VersioncontrolAuthHandlerMappedAccounts instance. Instanciate a new object.', E_RECOVERABLE_ERROR);
    }

    $this->repository = $repository;
    $this->build();
  }

  protected function build() {
    if ($this->built) {
      return; // already built, bail out
    }
    if (!$this->repository instanceof VersioncontrolRepository) {
      throw new Exception('Cannot build the account mapper object until a repository has been attached.');
    }

    // Retrieve the base auth data
    $this->userData = db_select('versioncontrol_auth_account', 'base')
      ->fields('base')
      ->condition('repo_id', $this->repository->repo_id)
      ->execute()
      ->fetchAllAssoc('uid', PDO::FETCH_ASSOC);

    foreach ($this->userData as &$data) {
      $data['per-label'] = array();
    }

    // Retrieve the extended per-label auth data
    $label_data = db_select('versioncontrol_auth_account_label', 'base')
      ->fields('base')
      ->condition('repo_id', $this->repository->repo_id)
      ->execute();

    foreach ($label_data as $row) {
      $labeldata = array(
        'label_update' => $row->label_update,
        'label_delete' => $row->label_delete,
      );
      $this->userData[$row->uid]['per-label'][$row->label_id] = $labeldata;
    }

    $this->built = TRUE;
  }

  public function authAccess($uid) {
    $this->build();
    if (empty($this->userData[$uid]) || empty($this->userData[$uid]['access'])) {
      // No account is registered, or access is set to 0 on the account
      $this->errors[] = t('User does not have access to this repository.');
      return FALSE;
    }
    return TRUE;
  }

  /**
   * Helper method - determine the base level of access to the repository for
   * the specified user.
   *
   * @param int $uid
   *   The uid of the Drupal user to be checked.
   * @return int
   *   The base access level for the specified user, or 0 if not found.
   */
  protected function baseAuth($uid) {
    $this->build();
    if (empty($this->userData[$uid])) {
      // If no record of the user, deny.
      return self::DENY;
    }
    else {
      return (int) $this->userData[$uid]['access'];
    }
  }

  public function authBranchCreate($uid, VersioncontrolBranch $branch) {
    $base = $this->baseAuth($uid);
    if ($base == self::DENY) {
      // Zero access, deny.
      $this->errors[] = t('User does not have access to create branches on this repository.');
      return FALSE;
    }
    else if ($base == self::ALL) {
      // User has super cow powers, say yes.
      return TRUE;
    }
    $access = $this->userData[$uid]['branch_create'] == self::GRANT;
    if (!$access) {
      $this->errors[] = t('User does not have access to create branches on this repository.');
    }
    return $access;
  }

  public function authBranchDelete($uid, VersioncontrolBranch $branch) {
    $access = $this->authLabel($uid, $branch, 'delete');
    if (!$access) {
      $this->errors[] = t('User does not have access to delete branches on this repository.');
    }
    return $access;
  }

  public function authBranchUpdate($uid, VersioncontrolBranch $branch) {
    $access = $this->authLabel($uid, $branch, 'update');
    if (!$access) {
      $this->errors[] = t('User does not have access to update branches on this repository.');
    }
    return $access;
  }
  public function authTagCreate($uid, VersioncontrolTag $tag) {
    $base = $this->baseAuth($uid);
    if ($base == self::DENY) {
      // Zero access, deny.
      $this->errors[] = t('User does not have access to create tags on this repository.');
      return FALSE;
    }
    else if ($base == self::ALL) {
      // User has super cow powers, say yes.
      return TRUE;
    }

    $access = $this->userData[$uid]['tag_create'] == self::GRANT;
    if (!$access) {
      $this->errors[] = t('User does not have access to create tags on this repository.');
    }
    return $access;
  }
  public function authTagDelete($uid, VersioncontrolTag $tag) {
    $access = $this->authLabel($uid, $tag, 'delete');
    if (!$access) {
      $this->errors[] = t('User does not have access to delete tags on this repository.');
    }
    return $access;
  }
  public function authTagUpdate($uid, VersioncontrolTag $tag) {
    $access = $this->authLabel($uid, $tag, 'update');
    if (!$access) {
      $this->errors[] = t('User does not have access to update tags on this repository.');
    }
    return $access;
  }

  /**
   * Perform an authorization check on a specified user against a specified
   * label for a specified op.
   *
   * This is just a shared helper method for the branch/tag update/delete
   * methods, as they all have virtually identical logical flow.
   *
   * @param int $uid
   *   The uid of the Drupal user to be checked.
   * @param VersioncontrolEntity $label
   *   Either a VersioncontrolTag or VersioncontrolBranch object, representing
   *   the label against which authorization checks should be made.
   * @param string $op
   *   Either 'update' or 'delete'.
   *
   * @return bool
   *   Boolean indicating access approved (TRUE) or denied (FALSE)
   */
  protected function authLabel($uid, VersioncontrolEntity $label, $op) {
    $base = $this->baseAuth($uid);
    switch ($base) {
      case self::DENY:
        // Zero access, deny.
        return FALSE;
      case self::ALL:
        // User has super cow powers, say yes.
        return TRUE;
    }

    $type = $label instanceof VersioncontrolTag ? 'tag' : 'branch';

    switch ($this->userData[$uid][$type . '_' . $op]) {
      case self::DENY:
        // User has no perms for this op on this label type
        return FALSE;
      case self::ALL:
        // User has all perms for this op on this label type
        return TRUE;
    }

    // If we get this far, then we're doing a label-specific perm check.
    return $this->userData[$uid]['per-label'][$label->label_id]['label_' . $op] == self::GRANT;
  }

  public function getErrorMessages() {
    return $this->errors;
  }

  /**
   * Set the permissions on this repository for the specified user.
   *
   * @param int $uid
   *   The uid to which the permissions data should be assigned.
   *
   * @param array $data
   *   The array of permissions data to be assigned.
   */
  public function setUserData($uid, $data) {
    $this->userData[$uid] = $data;
  }

  /**
   * Remove all permissions for a user. No data will be saved for this user.
   *
   * @param int $uid
   *   The uid for which permissions will be unset.
   */
  public function deleteUserData($uid) {
    unset($this->userData[$uid]);
  }

  /**
   * Retrieve the data representing a particular user's permission set, or the
   * entire set of permissions that have been set up for this repository.
   *
   * @param mixed $uid
   *   Permissions data will be retrieved for this uid. If not provided, all
   *   permissions data is returned.
   *
   * @return mixed
   *   An array of perm data for the requested user, or an array of such arrays
   *   keyed on uid. If an invalid user is requested, returns FALSE.
   */
  public function getUserData($uid = NULL) {
    if (is_null($uid)) {
      return $this->userData;
    }
    else {
      return empty($this->userData[$uid]) ? FALSE : $this->userData[$uid];
    }
  }

  /**
   * Save all auth information for the attached repository into the db.
   *
   * This operates by simply blowing away all data and rewriting it with mass
   * inserts, making it more performant overall but also leaving a very, very
   * small window during which auths may fail because the data is unavailable.
   */
  public function save() {
    if (!isset($this->repository)) {
      throw new Exception('Cannot save auth data without a repository to attach to.', E_ERROR);
    }
    db_delete('versioncontrol_auth_account')
      ->condition('repo_id', $this->repository->repo_id)
      ->execute();
    db_delete('versioncontrol_auth_account_label')
      ->condition('repo_id', $this->repository->repo_id)
      ->execute();

    // Prepare values
    $base_values = array();
    $per_label_values = array();
    foreach ($this->userData as $uid => $data) {
      $data['uid'] = $uid;
      $data['repo_id'] = $this->repository->repo_id;

      foreach ($data['per-label'] as $label_id => $label_data) {
        $label_data['uid'] = $uid;
        $label_data['repo_id'] = $this->repository->repo_id;
        $label_data['label_id'] = $label_id;

        $per_label_values[] = $label_data;
      }

      unset($data['per-label']);
      $base_values[] = $data;
    }

    // Perform base insert
    $fields = array('uid', 'repo_id', 'access', 'branch_create',
      'branch_update', 'branch_delete', 'tag_create', 'tag_update',
      'tag_delete');

    $insert = db_insert('versioncontrol_auth_account')->fields($fields);

    foreach ($base_values as $record) {
      $insert->values($record);
    }
    $insert->execute();

    $fields = array('uid', 'repo_id', 'label_id', 'label_update', 'label_delete');
    $insert = db_insert('versioncontrol_auth_account_label')->fields($fields);

    foreach ($per_label_values as $record) {
      $insert->values($record);
    }
    $insert->execute();
  }
}
