<?php
/**
 * @file
 * Operation class
 */

/**
 * Stuff that happened in a repository at a specific time
 */
abstract class VersioncontrolOperation extends VersioncontrolEntity {
  protected $_id = 'vc_op_id';
  /**
   * db identifier
   *
   * The Drupal-specific operation identifier (a simple integer)
   * which is unique among all operations (commits, branch ops, tag ops)
   * in all repositories.
   *
   * @var int
   */
  public $vc_op_id;

  /**
   * The time when the operation was performed, given as
   * Unix timestamp.
   *
   * @var timestamp
   */
  public $author_date;

  /**
   * The time when the operation was added to the repository, given as
   * Unix timestamp.
   *
   * @var timestamp
   */
  public $committer_date;

  /**
   * The VCS specific repository-wide revision identifier,
   * like '' in CVS, '27491' in Subversion or some SHA-1 key in various
   * distributed version control systems. If there is no such revision
   * (which may be the case for version control systems that don't support
   * atomic commits) then the 'revision' element is an empty string.
   * For branch and tag operations, this element indicates the
   * (repository-wide) revision of the files that were branched or tagged.
   *
   * @var string
   */
  public $revision;

  /**
   * The log message for the commit, tag or branch operation.
   * If a version control system doesn't support messages for the current
   * operation type, this element should be empty.
   *
   * @var string
   */
  public $message;

  /**
   * The Drupal user id of the operation author, or 0 if no Drupal user
   * could be associated to the author.
   *
   * @var int
   */
  public $author_uid;

  /**
   * The Drupal user id of the operation committer, or 0 if no Drupal user
   * could be associated to the author.
   *
   * @var int
   */
  public $committer_uid;

  /**
   * The system specific VCS username of the user who executed this
   * operation(aka who write the change)
   *
   * @var string
   */
  public $author;

  /**
   * Who actually perform the change on the repository.
   *
   * @var string
   */
  public $committer;

  /**
   * The type of the operation - one of the
   * VERSIONCONTROL_OPERATION_{COMMIT,BRANCH,TAG} constants.
   *
   * @var string
   */
  public $type;

  /**
   * An array of branches or tags that were affected by this
   * operation. Branch and tag operations are known to only affect one
   * branch or tag, so for these there will be only one element (with 0
   * as key) in 'labels'. Commits might affect any number of branches,
   * including none. Commits that emulate branches and/or tags (like
   * in Subversion, where they're not a native concept) can also include
   * add/delete/move operations for labels, as detailed below.
   * Mind that the main development branch - e.g. 'HEAD', 'trunk'
   * or 'master' - is also considered a branch. Each element in 'labels'
   * is a VersioncontrolLabel(VersioncontrolBranch VersioncontrolTag)
   *
   * @var array
   */
  public $labels = array();

  /**
   * An array of VersioncontrolItem objects affected by this commit.
   *
   * @var array
   */
  public $itemRevisions = array();

  protected $defaultCrudOptions = array(
    'update' => array('nested' => TRUE, 'map users' => FALSE),
    'insert' => array('nested' => TRUE, 'map users' => FALSE),
    'delete' => array('nested' => TRUE),
  );

  public function loadItemRevisions($ids = array(), $conditions = array(), $options = array()) {
    $conditions['repo_id'] = $this->repo_id;
    $conditions['vc_op_id'] = $this->vc_op_id;
    return $this->backend->loadEntities('item', $ids, $conditions, $options);
  }

  /**
   * Map author and committer data using the containing repository's mapping
   * plugin.
   *
   * @return bool
   *   TRUE if either mapping succeeded, FALSE if both failed.
   */
  public function mapUsers() {
    $succeeded = $this->mapAuthor();
    $succeeded = $this->mapCommitter() ? TRUE : $succeeded;
  }

  /**
   * Perform the mapping between Drupal users and this commit's author.
   *
   * @return bool
   *   TRUE if the mapping succeeded, FALSE otherwise.
   */
  public function mapAuthor() {
    if ($mapper = $this->repository->getAuthorMapper()) {
      $uid = $mapper->mapAuthor($this);
      $this->author_uid = empty($uid) ? 0 : $uid;
      return TRUE;
    }
    else {
      $this->author_uid = 0;
      return FALSE;
    }
  }

  /**
   * Perform the mapping between Drupal users and this commit's committer.
   *
   * @return bool
   *   TRUE if the mapping succeeded, FALSE otherwise.
   */
  public function mapCommitter() {
    if ($mapper = $this->repository->getCommitterMapper()) {
      $uid = $mapper->mapCommitter($this);
      $this->committer_uid = empty($uid) ? 0 : $uid;
      return TRUE;
    }
    else {
      $this->committer_uid = 0;
      return FALSE;
    }
  }

  public function insert($options = array()) {
    if (!empty($this->vc_op_id)) {
      // This is supposed to be a new commit, but has a vc_op_id already.
      throw new Exception('Attempted to insert a Versioncontrol commit which is already present in the database.', E_ERROR);
    }

    // Append default options.
    $options += $this->defaultCrudOptions['insert'];

    if ($options['map users']) {
      $this->mapUsers();
    }

    // make sure repo id is set for drupal_write_record()
    if (empty($this->repo_id)) {
      $this->repo_id = $this->repository->repo_id;
    }
    drupal_write_record('versioncontrol_operations', $this);

    if (!empty($options['nested'])) {
      $this->insertNested();
    }

    $this->backendInsert($options);

    // Everything's done, invoke the hook.
    module_invoke_all('versioncontrol_entity_commit_insert', $this);
    return $this;
  }

  protected function insertNested() {
    foreach ($this->itemRevisions as $item) {
      if (!isset($item->vc_op_id)) {
        $item->vc_op_id = $this->vc_op_id;
      }
      $item->insert(array('source item update' => TRUE));
    }
    $this->updateLabels();
  }

  public function update($options = array()) {
    if (empty($this->vc_op_id)) {
      // This is supposed to be an existing branch, but has no vc_op_id.
      throw new Exception('Attempted to update a Versioncontrol commit which has not yet been inserted in the database.', E_ERROR);
    }

    // Append default options.
    $options += $this->defaultCrudOptions['update'];

    if ($options['map users']) {
      $this->mapUsers();
    }

    // make sure repo id is set for drupal_write_record()
    if (empty($this->repo_id)) {
      $this->repo_id = $this->repository->repo_id;
    }
    drupal_write_record('versioncontrol_operations', $this, 'vc_op_id');

    if (!empty($options['nested'])) {
      $this->updateNested();
    }

    $this->backendUpdate($options);

    // Everything's done, invoke the hook.
    module_invoke_all('versioncontrol_entity_commit_update', $this);
    return $this;
  }

  protected function updateNested() {
    foreach ($this->itemRevisions as $item) {
      $item->save(array('source item update' => TRUE));
    }
    $this->updateLabels();
  }

  public function updateLabels() {
    db_delete('versioncontrol_operation_labels')
      ->condition('vc_op_id', $this->vc_op_id)
      ->execute();

    $insert = db_insert('versioncontrol_operation_labels')
      ->fields(array('vc_op_id', 'label_id', 'action'));
    foreach ($this->labels as $label) {
      // first, ensure there's a record of the label already
      if (!isset($label->label_id)) {
        $label->insert();
      }
      $values = array(
        'vc_op_id' => $this->vc_op_id,
        'label_id' => $label->label_id,
        // FIXME temporary hack, sets a default action. _CHANGE_ this.
        'action' => !empty($label->action) ? $label->action : VERSIONCONTROL_ACTION_MODIFIED,
      );
      $insert->values($values);
    }
    $insert->execute();
  }

  /**
   * Delete a commit, a branch operation or a tag operation from the database,
   * and call the necessary hooks.
   *
   * @param $operation
   *   The commit, branch operation or tag operation array containing
   *   the operation that should be deleted.
   */
  public function delete($options = array()) {
    // Append default options.
    $options += $this->defaultCrudOptions['delete'];

    db_delete('versioncontrol_operations')
      ->condition('vc_op_id', $this->vc_op_id)
      ->execute();

    if (!empty($options['nested'])) {
      $this->deleteNested($options);
    }

    // Remove relevant entries from the versioncontrol_operation_labels table.
    db_delete('versioncontrol_operation_labels')
      ->condition('vc_op_id', $this->vc_op_id)
      ->execute();

    $this->backendDelete($options);

    module_invoke_all('versioncontrol_entity_commit_delete', $this);
  }

  protected function deleteNested($options) {
    $items = $this->loadItemRevisions();
    foreach ($items as $item) {
      $item->delete($options);
    }
  }

  /**
   * Convinience method to call backend analogue one.
   *
   * @param $format
   *   Either 'full' for the original version, or 'short' for a more compact form.
   *   If the commit identifier doesn't need to be shortened, the results can
   *   be the same for both versions.
   */
  public function formatRevisionIdentifier($format = 'full') {
    return $this->backend->formatRevisionIdentifier($this->revision, $format);
  }

  /**
   * Retrieve the tag or branch that applied to that item during the
   * given operation. The result of this function will be used for the
   * selected label property of the item, which is necessary to preserve
   * the item state throughout navigational API functions.
   *
   * @param $item
   *   The item revision for which the label should be retrieved.
   *
   * @return
   *   NULL if the given item does not belong to any label or if the
   *   appropriate label cannot be retrieved. Otherwise a
   *   VersioncontrolLabel array is returned
   *
   *   In case the label array also contains the 'label_id' element
   *   (which happens when it's copied from the $operation->labels
   *   array) there will be a small performance improvement as the label
   *   doesn't need to be compared to and loaded from the database
   *   anymore.
   */
  public abstract function getSelectedLabel($item);
}
