<?php
/**
 * @file
 * Version Control API - An interface to version control systems
 * whose functionality is provided by pluggable back-end modules.
 *
 * This file contains module hooks for users of Version Control API,
 * with API documentation and a bit of example code.
 * Hooks that are intended for VCS backends are not to be found in this file
 * as they are already documented in versioncontrol_fakevcs.module.
 *
 * Copyright 2007, 2008, 2009 by Jakob Petsovits ("jpetso", http://drupal.org/user/56020)
 */


/**
 * Act on database changes when commit, tag or branch operations are inserted
 * or deleted. Note that this hook is not necessarily called at the time
 * when the operation actually happens - operations can also be inserted
 * by a cron script when the actual commit/branch/tag has been accomplished
 * for quite a while already.
 *
 * @param $op
 *   'insert' when the operation has just been recorded and inserted into the
 *   database, or 'delete' if it will be deleted right after this hook
 *   has been called.
 *
 * @param $operation
 *   An operation array containing basic information about the commit, branch
 *   or tag operation. It consists of the following elements:
 *
 *   - 'vc_op_id': The Drupal-specific operation identifier (a simple integer)
 *        which is unique among all operations (commits, branch ops, tag ops)
 *        in all repositories.
 *   - 'type': The type of the operation - one of the
 *        VERSIONCONTROL_OPERATION_{COMMIT,BRANCH,TAG} constants.
 *        Note that if you pass branch or tag constraints, this function might
 *        nevertheless return commit operations too - that happens for version
 *        control systems without native branches or tags (like Subversion)
 *        when a branch or tag is affected by the commit.
 *   - 'repository': The repository where this operation occurred.
 *        This is a structured "repository array", like is returned
 *        by versioncontrol_get_repository().
 *   - 'date': The time when the operation was performed, given as
 *        Unix timestamp. (For commits, this is the time when the revision
 *        was committed, whereas for branch/tag operations it is the time
 *        when the files were branched or tagged.)
 *   - 'uid': The Drupal user id of the operation author, or 0 if no
 *        Drupal user could be associated to the author.
 *   - 'username': The system specific VCS username of the author.
 *   - 'message': The log message for the commit, tag or branch operation.
 *        If a version control system doesn't support messages for any of them,
 *        this element contains an empty string.
 *   - 'revision': The VCS specific repository-wide revision identifier,
 *        like '' in CVS, '27491' in Subversion or some SHA-1 key in various
 *        distributed version control systems. If there is no such revision
 *        (which may be the case for version control systems that don't support
 *        atomic commits) then the 'revision' element is an empty string.
 *        For branch and tag operations, this element indicates the
 *        (repository-wide) revision of the files that were branched or tagged.
 *
 *   - 'labels': An array of branches or tags that were affected by this
 *        operation. Branch and tag operations are known to only affect one
 *        branch or tag, so for these there will be only one element (with 0
 *        as key) in 'labels'. Commits might affect any number of branches,
 *        including none. Commits that emulate branches and/or tags (like
 *        in Subversion, where they're not a native concept) can also include
 *        add/delete/move operations for labels, as detailed below.
 *        Mind that the main development branch - e.g. 'HEAD', 'trunk'
 *        or 'master' - is also considered a branch. Each element in 'labels'
 *        is a structured array with the following keys:
 *
 *        - 'id': The label identifier (a simple integer), used for unique
 *             identification of branches and tags in the database.
 *        - 'name': The branch or tag name (a string).
 *        - 'action': Specifies what happened to this label in this operation.
 *             For plain commits, this is always VERSIONCONTROL_ACTION_MODIFIED.
 *             For branch or tag operations (or commits that emulate those),
 *             it can be either VERSIONCONTROL_ACTION_ADDED or
 *             VERSIONCONTROL_ACTION_DELETED.
 *
 * @param $operation_items
 *   A structured array containing all items that were affected by the above
 *   operation. Array keys are the current/new paths, even if the item doesn't
 *   exist anymore (as is the case with delete actions in commits).
 *   The associated array elements are structured item arrays and consist of
 *   the following elements:
 *
 *   - 'type': Specifies the item type, which is either
 *        VERSIONCONTROL_ITEM_FILE or VERSIONCONTROL_ITEM_DIRECTORY for items
 *        that still exist, or VERSIONCONTROL_ITEM_FILE_DELETED respectively
 *        VERSIONCONTROL_ITEM_DIRECTORY_DELETED for items that have been
 *        removed (by a commit's delete action).
 *   - 'path': The path of the item at the specific revision.
 *   - 'revision': The (file-level) revision when the item was changed.
 *        If there is no such revision (which may be the case for
 *        directory items) then the 'revision' element is an empty string.
 *   - 'item_revision_id': Identifier of this item revision in the database.
 *        Note that you can only rely on this element to exist for
 *        operation items - functions that interface directly with the VCS
 *        (such as VersioncontrolItem::getDirectoryContents() or
 *        VersioncontrolItem::getParallelItems()) might not include
 *        this identifier, for obvious reasons.
 *
 *   For commit operations, additional information about the origin of
 *   the items is also available. The following elements will be set
 *   for each item in addition to the ones listed above:
 *
 *   - 'action': Specifies how the item was changed.
 *        One of the predefined VERSIONCONTROL_ACTION_* values.
 *   - 'source_items': An array with the previous state(s) of the affected item.
 *        Empty if 'action' is VERSIONCONTROL_ACTION_ADDED.
 *   - 'replaced_item': The previous but technically unrelated item at the
 *        same location as the current item. Only exists if this previous item
 *        was deleted and replaced by a different one that was just moved
 *        or copied to this location.
 *
 * @ingroup Operations
 * @ingroup Commit notification
 * @ingroup Database change notification
 */
function hook_versioncontrol_operation($op, $operation, $operation_items) {
  if ($op == 'insert' && module_exists('commitlog')) {
    if (variable_get('commitlog_send_notification_mails', 0)) {
      $mailto = variable_get('versioncontrol_email_address', 'versioncontrol@example.com');
      commitlog_notification_mail($mailto, $operation, $operation_items);
    }
  }
}

/**
 * Act on database changes when operation labels change or a given operation.
 *
 * @param $op
 *   'insert' when the operation along with its label associations has just
 *   been recorded and inserted into the database, 'update' when the set of
 *   operation labels changes, or 'delete' if the operation along with its
 *   label associations. Note that updating or deleting an operation label
 *   does not automatically trigger deletion of the label itself in the
 *   database, just the association of the operation to the label.
 *
 * @param $operation
 *   An operation array containing basic information about the commit, branch
 *   or tag operation. This is the same operation array format as is passed
 *   to hook_versioncontrol_operation() (and other functions), see the
 *   API documentation there for an exact description of its properties.
 *
 *   The operation array holds the labels' state *before* the action is being
 *   performed. That means when @p $op is 'insert', $operation['labels'] is an
 *   empty array, whereas with @p $op being 'update' or 'delete',
 *   $operation['labels'] holds the previous set of operation labels (which may
 *   also be empty of course).
 *
 * @param $labels
 *   The new set of operation labels - an array of branches or tags that were
 *   affected by the given operation. When @p $op is 'delete', this is an empty
 *   array, whereas with @p $op being 'insert' or 'update', @p $labels holds
 *   the new set of operation labels (which may also be empty of course).
 *   Same format as $operation['labels'].
 *
 * @ingroup Operations
 * @ingroup Database change notification
 */
function hook_versioncontrol_operation_labels($op, $operation, $labels) {
  // This crude example tracks which labels are being added and removed, and
  // adjusts a counter accordingly. Untested, don't assume this actually works.
  $old_label_ids = $new_label_ids = array();
  $adjustment_operators = array();

  foreach ($operation['labels'] as $label) {
    $old_label_ids[] = $label['label_id'];
  }
  foreach ($labels as $label) {
    if (!in_array($label['label_id'], $old_label_ids)) {
      $adjustment_operators[$label['label_id']] = '+';
    }
    $new_label_ids[] = $label['label_id'];
  }
  foreach ($old_label_ids as $old_label_id) {
    if (!in_array($old_label_id, $new_label_ids)) {
      $adjustment_operators[$old_label_id] = '-';
    }
  }

  foreach ($adjustment_operators as $label_id => $operator) {
    $result = db_query('SELECT label_id FROM {mymodule_label_activity}');
    if (!db_result($result)) { // does not yet exist in the database
      db_query("INSERT INTO {mymodule_label_activity} (label_id, activity)
                VALUES (%d, 0)", $label_id);
    }
    db_query("UPDATE {mymodule_label_activity}
              SET activity = activity $operator 1
              WHERE label_id = %d", $label_id);
  }
}

/**
 * Extract repository data from the repository edit/add form's submitted
 * values, and add it to the @p $repository array. Later, that array will be
 * passed to hook_versioncontrol_repository() as part of the repository
 * insert/update procedure.
 *
 * Elements written to $repository['data'][$module] will be automatically
 * serialized and stored with the repository, you can write to that array
 * in order to store module-specific repository settings. If there are settings
 * that require a lot of memory or need to be accessible for SQL queries, you
 * might be better off storing these in your own module's table with
 * hook_versioncontrol_repository().
 *
 * @param $repository
 *   The repository array which is being passed by reference so that it can be
 *   written to.
 * @param $form
 *   The form array of the submitted repository edit/add form, with
 *   $form['#id'] == 'versioncontrol-repository-form' (amongst others).
 * @param $form_state
 *   The form state of the submitted repository edit/add form.
 *   If you altered this form and added an additional form element then
 *   $form_state['values'] will also contain the value of this form element.
 *
 * @ingroup Repositories
 * @ingroup Form handling
 * @ingroup Target audience: All modules with repository specific settings
 */
function hook_versioncontrol_repository_submit(&$repository, $form, $form_state) {
  // The user can specify multiple repository ponies, separated by whitespace.
  // So, split the string up into an array of ponies.
  $ponies = trim($form_state['values']['mymodule_ponies']);
  $ponies = empty($ponies) ? array() : explode(' ', $ponies);
  $repository['mymodule']['ponies'] = $ponies;
}

/**
 * Act on database changes when VCS repositories are inserted,
 * updated or deleted.
 *
 * @param $op
 *   Either 'insert' when the repository has just been created, or 'update'
 *   when repository name, root, URL backend or module specific data change,
 *   or 'delete' if it will be deleted after this function has been called.
 *
 * @param $repository
 *   The repository array containing the repository. It's a single
 *   repository array like the one returned by versioncontrol_get_repository(),
 *   so it consists of the following elements:
 *
 *   - 'repo_id': The unique repository id.
 *   - 'name': The user-visible name of the repository.
 *   - 'vcs': The unique string identifier of the version control system
 *        that powers this repository.
 *   - 'root': The root directory of the repository. In most cases,
 *        this will be a local directory (e.g. '/var/repos/drupal'),
 *        but it may also be some specialized string for remote repository
 *        access. How this string may look like depends on the backend.
 *   - 'authorization_method': The string identifier of the repository's
 *        authorization method, that is, how users may register accounts
 *        in this repository. Modules can provide their own methods
 *        by implementing hook_versioncontrol_authorization_methods().
 *   - 'data': An array where modules can store additional information about
 *        the repository, for settings or other data.
 *   - '[xxx]_specific': An array of VCS specific additional repository
 *        information. How this array looks like is defined by the
 *        corresponding backend module (versioncontrol_[xxx]).
 *        (Deprecated, to be replaced by the more general 'data' property.)
 *   - '???': Any other additions that modules added by implementing
 *        hook_versioncontrol_repository_submit().
 *
 * @ingroup Repositories
 * @ingroup Database change notification
 * @ingroup Form handling
 * @ingroup Target audience: All modules with repository specific settings
 */
function hook_versioncontrol_repository($op, $repository) {
  $ponies = $repository['mymodule']['ponies'];

  switch ($op) {
    case 'update':
      db_query("DELETE FROM {mymodule_ponies}
                WHERE repo_id = %d", $repository['repo_id']);
      // fall through
    case 'insert':
      foreach ($ponies as $pony) {
        db_query("INSERT INTO {mymodule_ponies} (repo_id, pony)
                  VALUES (%d, %s)", $repository['repo_id'], $pony);
      }
      break;

    case 'delete':
      db_query("DELETE FROM {mymodule_ponies}
                WHERE repo_id = %d", $repository['repo_id']);
      break;
  }
}


/**
 * Register new authorization methods that can be selected for a repository.
 * A module may restrict access and alter forms depending on the selected
 * authorization method which is a property of every repository array
 * ($repository['authorization_method']).
 *
 * A list of all authorization methods can be retrieved
 * by calling versioncontrol_get_authorization_methods().
 *
 * @return
 *   A structured array containing information about authorization methods
 *   provided by this module, wrapped in a structured array. Array keys are
 *   the unique string identifiers of each authorization method, and
 *   array values are the user-visible method descriptions (wrapped in t()).
 *
 * @ingroup Accounts
 * @ingroup Authorization
 * @ingroup Target audience: Authorization control modules
 */
function hook_versioncontrol_authorization_methods() {
  return array(
    'mymodule_code_ninja' => t('Code ninja skills required'),
  );
}

/**
 * Alter the list of repositories that are available for user registration
 * and editing.
 *
 * @param $repository_names
 *   The list of repository names as it is shown in the select box
 *   at 'versioncontrol/register'. Array keys are the repository ids,
 *   and array elements are the captions in the select box.
 *   There's two things that can be done with this array:
 *   - Change (amend) the caption, in order to provide more information
 *     for the user. (E.g. note that an application is necessary.)
 *   - Unset any number of array elements. If you do so, the user will not
 *     be able to register a new account for this repository.
 * @param $repositories
 *   A list of repositories (with the repository ids as array keys) that
 *   includes at least all of the repositories that correspond to the
 *   repository ids of the @p $repository_names array.
 *
 * @ingroup Accounts
 * @ingroup Authorization
 * @ingroup Repositories
 * @ingroup Form handling
 * @ingroup Target audience: Authorization control modules
 */
function hook_versioncontrol_alter_repository_selection(&$repository_names, $repositories) {
  global $user;

  foreach ($repository_names as $repo_id => $caption) {
    if ($repositories[$repo_id]['authorization_method'] == 'mymodule_code_ninja') {
      if (!in_array('code ninja', $user->roles)) {
        unset($repository_names[$repo_id]);
      }
    }
  }
}

/**
 * Let the Version Control API know whether the given VCS account
 * is authorized or not.
 *
 * @param $repository
 *   The repository where the status should be checked. (Note that the user's
 *   authorization status may differ for each repository.)
 * @param $uid
 *   The user id of the checked account.
 *
 * @return
 *   TRUE if the account is authorized, or FALSE if it's not.
 *
 * @ingroup Accounts
 * @ingroup Authorization
 * @ingroup Target audience: Authorization control modules
 */
function hook_versioncontrol_is_account_authorized($repository, $uid) {
  if ($repository['authorization_method'] != 'mymodule_dojo_status') {
    return TRUE;
  }
  $result = db_query("SELECT status
                      FROM {mymodule_dojo_status}
                      WHERE uid = %d AND repo_id = %d",
                      $uid, $repository['repo_id']);

  while ($account = db_fetch_object($result)) {
    return ($account->status == MYMODULE_SENSEI);
  }
  return FALSE;
}
