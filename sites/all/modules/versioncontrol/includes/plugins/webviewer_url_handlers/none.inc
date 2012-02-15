<?php
/**
 * @file
 * This plugin contains the main documentation of how to make a
 * webviewer_url_handler. It does not really provide any functionallity.
 */

$plugin = array(

  // The versioncontrol system machine name as the backend declares.
  'vcs' => 'test',

  // This title is going to be shown on the repository edition, for the
  // user to identify the plugin.
  'title' => t('Empty URL handler'),

  // The list of URL templates this webviewer. Leave empty the URL
  // template entry if the web viewer do not support that template.
  'url_templates' => array(

    /**
     * Template URL for the main page for this repository.
     *
     * It contains the following placeholders:
     * - "%base_url" the URL to the repository viewer.
     * - "%repo_name" the name of the repository on the filesystem.
     */
    'repository_view' => '',

    /**
     * Template url for the commit view.
     *
     * The URL of the repository viewer that displays a given commit in the
     * repository.
     * It contains the following placeholders:
     * - "%base_url" the URL to the repository viewer.
     * - "%repo_name" the name of the repository on the filesystem.
     * - "%revision" for the revision/commit/changeset identifier.
     */
    'commit_view' => '',

    /**
     * Template url for the file log view.
     *
     * The URL of the repository viewer that displays the commit log of
     * a given file in the repository.
     * It contains the following placeholders:
     * - "%base_url" the URL to the repository viewer.
     * - "%repo_name" the name of the repository on the filesystem.
     * - "%path" for the file path
     * - "%revision" will be replaced by the file-level revision (the one
     *   in {versioncontrol_item_revisions}.revision)
     * - "%branch" will be replaced by the branch name that the file is on
     */
    'file_log_view' => '',

    /**
     * Template url for the file directory log view.
     *
     * The URL of the repository viewer that displays the commit log of
     * a given directoryin the repository.
     * It contains the following placeholders:
     * - "%base_url" the URL to the repository viewer.
     * - "%repo_name" the name of the repository on the filesystem.
     * - "%path" for the file path
     * - "%revision" will be replaced by the file-level revision (the one
     *   in {versioncontrol_item_revisions}.revision)
     * - "%branch" will be replaced by the branch name that the file is on
     */
    'directory_log_view' => '',

    /**
     * Template url for the file view.
     *
     * The URL of the repository viewer that displays the contents of a
     * given file in the repository.
     * It contains the following placeholders:
     * - "%base_url" the URL to the repository viewer.
     * - "%repo_name" the name of the repository on the filesystem.
     * - "%path" for the file path
     * - "%revision" will be replaced by the file-level revision (the
     *   one in {versioncontrol_item_revisions}.revision)
     * - "%branch" will be replaced by the branch name that the file is on
     */
    'file_view' => '',

    /**
     * Template url for the directory view.
     *
     * The URL of the repository viewer that displays the contents of a given
     * directory in the repository.
     * It contains the following placeholders:
     * - "%base_url" the URL to the repository viewer.
     * - "%repo_name" the name of the repository on the filesystem.
     * - "%path" for the directory path
     * - "%revision" will be replaced by the file-level revision (the
     *   one in {versioncontrol_item_revisions}.revision - only makes sense
     *   if directories are versioned, of course)
     * - "%branch" will be replaced by the branch name that the
     *   directory is on.
     */
    'directory_view' => '',

    /**
     * Template url for the diff view.
     *
     * The URL of the repository viewer that displays the diff between two
     * given files in the repository.
     * It contains the following placeholders:
     * - "%base_url" the URL to the repository viewer.
     * - "%repo_name" the name of the repository on the filesystem.
     * - "%path" and "%old_path" for the new and old paths (for some
     *   version control systems, like CVS, those paths will always be
     *   the same).
     * - "%new_revision" and "%old_revision" will be replaced by the
     *   respective file-level revisions (from
     *   {versioncontrol_item_revisions}.revision)
     * - "%branch" will be replaced by the branch name that the file is on.
     */
    'diff' => '',

  ),
  'handler' => array(
    'class' => 'VersioncontrolRepositoryUrlHandler',
    'file' => 'VersioncontrolRepositoryUrlHandler.inc',
    'path' => drupal_get_path('module', 'versioncontrol') . '/includes/plugins/webviewer_url_handlers',
  ),
);
