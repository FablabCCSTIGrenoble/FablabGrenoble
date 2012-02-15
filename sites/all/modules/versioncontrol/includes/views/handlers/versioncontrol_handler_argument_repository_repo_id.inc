<?php
// $Id$
/**
 * @file
 * Contains the repo_id argument handler.
 */

/**
 * Specialized argument handler for repo_id argument.
 */
class versioncontrol_handler_argument_repository_repo_id extends views_handler_argument_numeric {
  function title_query() {
    $titles = array();
    $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));

    $result = db_query("SELECT vcr.name FROM {versioncontrol_repositories} vcr WHERE vcr.repo_id IN ($placeholders)", $this->value);
    while ($repo = db_fetch_object($result)) {
      $titles[] = check_plain($repo->name);
    }
    return $titles;
  }
}
