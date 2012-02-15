<?php

/**
 * @file
 *
 * Implement views hooks to provide views integration for Version Control API's
 * tables and define the requisite plugins for doing so.
 */

/**
 * Implementation of hook_views_data().
 *
 * Describes schema for versioncontrol tables.
 *
 * @return array
 */
function versioncontrol_views_data() {
  $data = array();

  // VersionControl Repositories
  $data['versioncontrol_repositories']['table']['group'] = t("VersionControl Repository");
  $data['versioncontrol_repositories']['table']['base'] = array(
    'field' => 'repo_id',
    'title' => t('VersionControl Repository'),
    'help'  => t('The repositories tracked by VersionControl.'),
  );
  $data['versioncontrol_repositories']['repo_id'] = array(
    'title' => t("Repository ID"),
    'help' => t("The unique identifier of the repository"),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'argument' => array(
      'handler' => 'versioncontrol_handler_argument_repository_repo_id',
    ),
  );
  $data['versioncontrol_repositories']['name'] = array(
    'title' => t("Repository Name"),
    'help' => t("The name of the repository"),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['versioncontrol_repositories']['vcs'] = array(
    'title' => t("Backend Type"),
    'help' => t("The backend type of VCS that this repository uses"),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'versioncontrol_handler_filter_repo_backend',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['versioncontrol_repositories']['update_method'] = array(
    'title' => t("Update method"),
    'help' => t("The method by which this repository will be updated."),
    'field' => array(
      'handler' => 'versioncontrol_handler_field_repository_update_method',
      'click sortable' => TRUE,
    ),
  );

  $data['versioncontrol_repositories']['actions'] = array(
    'title' => t("Actions"),
    'help' => t("Actions that can be performed on a repository."),
    'field' => array(
      'handler' => 'versioncontrol_handler_field_repository_actions',
      'click sortable' => TRUE,
    ),
  );

  $data['versioncontrol_repositories']['manual_fetch'] = array(
    'title' => t("Log Fetching"),
    'help' => t("Controls for managing manual log fetching."),
    'field' => array(
      'handler' => 'versioncontrol_handler_field_repository_manual_fetch',
      'click sortable' => TRUE,
    ),
  );

  $data['versioncontrol_repositories']['root'] = array(
    'title' => t("Root Path"),
    'help' => t("The root path to the repository"),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter',
    ),
  );
  $data['versioncontrol_repositories']['updated'] = array(
    'title' => t("Last Updated"),
    'help' => t("The last time the repository was updated."),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );
  $data['versioncontrol_repositories']['table']['join'] = array(
    'versioncontrol_operations' => array(
      'left_field' => 'repo_id',
      'field' => 'repo_id',
      'type' => 'LEFT',
    ),
    'versioncontrol_item_revisions' => array(
      'left_field' => 'repo_id',
      'field' => 'repo_id',
      'type' => 'LEFT',
    ),
  );

  // VersionControl Operations
  $data['versioncontrol_operations']['table']['group'] = t('VersionControl Operations');
  $data['versioncontrol_operations']['table']['base'] = array(
    'field' => 'vc_op_id',
    'title' => t('VersionControl Operation'),
    'help'  => t('Operations are the actions that can be performed on a repository.  Examples are commit, tag, merge, and branch.'),
  );
  $data['versioncontrol_operations']['vc_op_id'] = array(
    'title'    => t('Operation ID'),
    'help'     => t('The unique ID for a operation'),
    'field'    => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );
  $data['versioncontrol_operations']['type'] = array(
    'title'    => t('Type'),
    'help'     => t('The type of operation. Tag, Commit or Branch'),
    'field'    => array(
      'handler' => 'versioncontrol_handler_field_operation_type',
       'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'versioncontrol_handler_filter_operation_type',
    ),
  );
  $data['versioncontrol_operations']['author_date'] = array(
    'title'    => t('Author date'),
    'help'     => t('The date when the operation was originally done.'),
    'field'    => array(
      'handler' => 'versioncontrol_handler_field_operation_date',
      'click sortable' => TRUE,
      'additional fields' => array(
        'date_value' => array('table' => 'versioncontrol_operations', 'field' => 'author_date'),
        'revision',
        'repo_id',
      ),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_date',
    ),
  );
  $data['versioncontrol_operations']['committer_date'] = array(
    'title'    => t('Committer date'),
    'help'     => t('The date when the operation was added to the repository.'),
    'field'    => array(
      'handler' => 'versioncontrol_handler_field_operation_date',
      'click sortable' => TRUE,
      'additional fields' => array(
        'date_value' => array('table' => 'versioncontrol_operations', 'field' => 'committer_date'),
        'revision',
        'repo_id',
      ),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_date',
    ),
  );

  $data['versioncontrol_operations']['author'] = array(
    'title'    => t('Author'),
    'help'     => t('The Operation Author'),
    'field'    => array(
      'handler' => 'versioncontrol_handler_field_operation_person',
      'click sortable' => TRUE,
      'additional fields' => array(
        'person_uid' => array('table' => 'versioncontrol_operations', 'field' => 'author_uid'),
        'person_username' => array('table' => 'users', 'field' => 'name'),
      ),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  $data['versioncontrol_operations']['attribution'] = array(
    'real field' => 'author',
    'title'    => t('Attribution'),
    'help'     => t('The operation author (and committer, if different)'),
    'field'    => array(
      'handler' => 'versioncontrol_handler_field_operation_attribution',
      'additional fields' => array(
        'author_uid', 'committer_uid', 'committer',
      ),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  $data['versioncontrol_operations']['author_uid'] = array(
    'title' => t('User'),
    'help' => t('The user that did the operation'),
    'relationship' => array(
      'base'  => 'users',
      'base field' => 'uid',
      'handler' => 'views_handler_relationship',
      'label' => t('User'),
    ),
  );

  $data['versioncontrol_operations']['committer'] = array(
    'title'    => t('Commiter'),
    'help'     => t('The Operation Commiter'),
    'field'    => array(
      'handler' => 'versioncontrol_handler_field_operation_person',
      'click sortable' => TRUE,
      'additional fields' => array(
        'person_uid' => array('table' => 'versioncontrol_operations', 'field' => 'committer_uid'),
        'person_username' => array('table' => 'users', 'field' => 'name'),
      ),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  $data['versioncontrol_operations']['committer_uid'] = array(
    'title' => t('User'),
    'help' => t('The user that did the operation'),
    'relationship' => array(
      'base'  => 'users',
      'base field' => 'uid',
      'handler' => 'views_handler_relationship',
      'label' => t('User'),
    ),
  );

  $data['versioncontrol_operations']['revision'] = array(
    'title'    => t('Revision'),
    'help'     => t('The Operation Revision Identifier'),
    'field'    => array(
      'handler' => 'versioncontrol_handler_field_operation_revision',
      'click sortable' => TRUE,
      'additional fields' => array(
        'vcs' => array('table' => 'versioncontrol_repositories', 'field' => 'vcs'),
        'repo_id' => array('table' => 'versioncontrol_repositories', 'field' => 'repo_id'),
      ),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['versioncontrol_operations']['message'] = array(
    'title'    => t('Message'),
    'help'     => t('The Operation Message'),
    'field'    => array(
      'handler' => 'versioncontrol_handler_field_operation_message',
    ),
  );

  $data['versioncontrol_operations']['labels'] = array(
    'real field' => 'vc_op_id',
    'title'    => t('List of label this operation is in'),
    'help'     => t('This operation labels as a list on label names.'),
    'field'    => array(
      'handler' => 'versioncontrol_handler_field_operation_labels',
      'additional fields' => array(
        'vcs' => array('table' => 'versioncontrol_repositories', 'field' => 'vcs'),
        'repo_id',
      ),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  $data['versioncontrol_operations']['table']['join'] = array(
    'users' => array(
      'left_field' => 'uid',
      'field'      => 'author_uid',
      'type' => 'LEFT',
    ),
    'versioncontrol_repositories' => array(
      'left_field' => 'repo_id',
      'field' => 'repo_id',
      'type' => 'LEFT',
    ),
    'versioncontrol_operation_labels' => array(
      'left_field' => 'vc_op_id',
      'field' => 'vc_op_id',
      'type' => 'LEFT',
    ),
    'versioncontrol_labels' => array(
      'left_table' => 'versioncontrol_operation_labels',
      'left_field' => 'vc_op_id',
      'field' => 'vc_op_id',
      'type' => 'LEFT',
    ),
    'versioncontrol_item_revisions' => array(
      'left_field' => 'vc_op_id',
      'field' => 'vc_op_id',
      'type' => 'LEFT',
    ),
  );
  $data['users']['table']['join']['versioncontrol_operations'] = array(
    'left_field' => 'author_uid',
      'field'      => 'uid',
      'type'       => 'LEFT',
  );

  // VersionControl Labels
  $data['versioncontrol_labels']['table']['group'] = t("VersionControl Labels");
  $data['versioncontrol_labels']['table']['base'] = array(
    'field' => 'label_id',
    'title' => t('VersionControl Branches/Tags'),
    'help'  => t('Branches & tags in repositories tracked by the Versioncontrol system.'),
  );
  $data['versioncontrol_labels']['table']['join'] = array(
    'versioncontrol_operation_labels' => array(
      'left_field' => 'label_id',
      'field' => 'label_id',
      'type' => 'LEFT',
    ),
    'versioncontrol_operations' => array(
      'left_table' => 'versioncontrol_operation_labels',
      'left_field' => 'label_id',
      'field' => 'label_id',
      'type' => 'LEFT',
    ),
  );
  $data['versioncontrol_labels']['label_id'] = array(
      'title' => t("Label ID"),
      'help' => t("The unique label identifier"),
      'field' => array(
        'handler' => 'views_handler_field_numeric',
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_numeric',
      ),
      'argument' => array(
        'handler' => 'views_handler_argument_numeric',
      ),
  );
  $data['versioncontrol_labels']['name'] = array(
    'title' => t("Label Name"),
    'help' => t("The name of this label"),
    'field' => array(
      'handler' => 'views_handler_field',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'versioncontrol_handler_filter_operation_type_label',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['versioncontrol_labels']['type'] = array(
      'title' => t('Label Type'),
      'help' => t('The label type - either a tag or a branch.'),
      //TODO: Handler 'field' and 'filter'
      'field' => array(
        'handler' => 'views_handler_field_numeric',
      ),
  );
  // VersionControl Operations Labels
  $data['versioncontrol_operation_labels']['table']['group'] = t("VersionControl Labels");
  $data['versioncontrol_operation_labels']['table']['join'] = array(
    'versioncontrol_labels' => array(
      'left_field' => 'label_id',
      'field' => 'label_id',
      'type' => 'LEFT',
    ),
    'versioncontrol_operations' => array(
      'left_field' => 'vc_op_id',
      'field' => 'vc_op_id',
      'type' => 'LEFT',
    ),
  );
  $data['versioncontrol_operation_labels']['action'] = array(
    //TODO: What is this again?
    'title' => t("Op Label Action"),
    'help' => t("The operations action on this label"),

    'field' => array(
      'handler' => 'views_handler_field_numeric',
    ),
  );

  // VersionControl Item Revisions
  $data['versioncontrol_item_revisions']['table']['group'] = t("VersionControl Item Revisions");
  $data['versioncontrol_item_revisions']['table']['base'] = array(
    'field' => 'item_revision_id',
    'title' => t('VersionControl Item Revisions'),
    'help'  => t('File-by-file revision histories.'),
  );
  $data['versioncontrol_item_revisions']['table']['join'] = array(
    'versioncontrol_operations' => array(
      'left_field' => 'vc_op_id',
      'field' => 'vc_op_id',
      'type' => 'LEFT',
    ),

    'versioncontrol_repositories' => array(
      'left_field' => 'repo_id',
      'field' => 'repo_id',
      'type' => 'LEFT',
    ),
  );
  $data['versioncontrol_item_revisions']['path'] = array(
    'title' => t("File path"),
    'help' => t("The file path within the repository"),
    'field'    => array(
      'handler' => 'versioncontrol_handler_field_item_revision_path',
      'click sortable' => TRUE,
      'additional fields' => array(
        'vcs' => array('table' => 'versioncontrol_repositories', 'field' => 'vcs'),
        'repo_id' => array('table' => 'versioncontrol_repositories', 'field' => 'repo_id'),
        'item_revision_id',
      ),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['versioncontrol_item_revisions']['revision'] = array(
    'title' => t("Revision ID"),
    'help' => t("The revision identifier for this item"),
    'field' => array(
      'handler' => 'versioncontrol_handler_field_operation_revision',
      'click sortable' => TRUE,
      'additional fields' => array(
        'vcs' => array('table' => 'versioncontrol_repositories', 'field' => 'vcs'),
        'repo_id' => array('table' => 'versioncontrol_repositories', 'field' => 'repo_id'),
      ),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['versioncontrol_item_revisions']['type'] = array(
    'title' => t("Op Item Type"),
    'help' => t("The type of item - file or directory & deleted or current."),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
    ),
  );
  // TODO: source_item_revision_id
  $data['versioncontrol_item_revisions']['source_item_revision_id'] = array();
  $data['versioncontrol_item_revisions']['action'] = array(
    'title' => t('Action'),
    'help' => t('The action performed on this item'),
    'field' => array(
      'handler' => 'versioncontrol_handler_field_item_revision_action',
    ),
  );
  $data['versioncontrol_item_revisions']['line_changes_added'] = array(
    'title' => t('Lines Added'),
    'help' => t('The number of lines added to this item'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  $data['versioncontrol_item_revisions']['line_changes_removed'] = array(
    'title' => t('Lines Removed'),
    'help' => t('The number of lines removed from this item'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  $data['versioncontrol_item_revisions']['changed_lines'] = array(
    'title' => t('Lines changed'),
    'help' => t('A single line representation of lines changed.'),
    'field' => array(
      'handler' => 'versioncontrol_handler_field_item_revision_lines_changed',
      'click sortable' => FALSE,
    ),
  );
  $data['versioncontrol_item_revisions']['visual_diffstat'] = array(
    'title' => t('Visual diffstat'),
    'help' => t('A visual represtation of the stasitics representing the difference between revisions of a file.'),
    'field' => array(
      'handler' => 'versioncontrol_handler_field_item_revision_visual_diffstat',
      'click sortable' => FALSE,
    ),
  );
  return $data;
}



/**
 * Implements hook_views_handler().
 *
 * @return array
 */
function versioncontrol_views_handlers() {
  $ret = array(
    'info' => array(
      'path'       => drupal_get_path('module', 'versioncontrol') . '/includes/views/handlers',
    ),
    'handlers' => array(
      // field handlers
      'versioncontrol_handler_field_repository_update_method' => array(
        'parent' => 'views_handler_field',
      ),
      'versioncontrol_handler_field_repository_manual_fetch' => array(
        'parent' => 'views_handler_field',
      ),
      'versioncontrol_handler_field_repository_actions' => array(
        'parent' => 'views_handler_field',
      ),
      'versioncontrol_handler_field_operation_type' => array(
        'parent' => 'views_handler_field',
      ),
      'versioncontrol_handler_field_operation_message' => array(
        'parent' => 'views_handler_field',
      ),
      'versioncontrol_handler_field_operation_revision' => array(
        'parent' => 'views_handler_field',
      ),
      'versioncontrol_handler_field_operation_person' => array(
        'parent' => 'views_handler_field',
      ),
      'versioncontrol_handler_field_operation_attribution' => array(
        'parent' => 'versioncontrol_handler_field_operation_person',
      ),
      'versioncontrol_handler_field_item_revision_action' => array(
        'parent' => 'views_handler_field',
      ),
      'versioncontrol_handler_field_item_revision_path' => array(
        'parent' => 'views_handler_field',
      ),
      'versioncontrol_handler_field_item_revision_visual_diffstat' => array(
        'parent' => 'views_handler_field',
      ),
      'versioncontrol_handler_field_item_revision_lines_changed' => array(
        'parent' => 'views_handler_field',
      ),
      'versioncontrol_handler_field_operation_date' => array(
        'parent' => 'views_handler_field_date',
      ),
      'versioncontrol_handler_field_operation_labels' => array(
        'parent' => 'views_handler_field',
      ),
      // filter handlers
      'versioncontrol_handler_filter_operation_type' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),
      'versioncontrol_handler_filter_repo_backend' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),
      'versioncontrol_handler_filter_operation_type_label' => array(
        'parent' => 'views_handler_filter_string',
      ),
      // argument handlers
      'versioncontrol_handler_argument_repository_repo_id' => array(
        'parent' => 'views_handler_argument_numeric',
      ),
    ),
  );
  return $ret;
}

/**
 * Implementation of hook_views_plugins.
 */
function versioncontrol_views_plugins() {
  return array(
    'row' => array(
      'versioncontrol_operations_rss' => array(
        'title' => t('Versioncontrol Operations as RSS items'),
        'help' => t('Display versioncontrol operations for RSS feeds.'),
        'handler' => 'versioncontrol_plugin_row_operation_rss',
        'path' => drupal_get_path('module', 'versioncontrol') . '/includes/views/plugins', // not necessary for most modules
        'theme' => 'versioncontrol_view_row_operation_rss',
        'theme file' => 'versioncontrol_theme.inc',
        'theme path' => drupal_get_path('module', 'versioncontrol') . '/includes',
        'base' => array('versioncontrol_operations'), // only works with 'versioncontrol_operations' as base.
        'uses options' => TRUE,
        'uses fields' => TRUE,
        'type' => 'feed',
        'help topic' => 'style-node-rss',
      ),
    ),
  );
}
