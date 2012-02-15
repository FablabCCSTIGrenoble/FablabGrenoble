<?php

/**
 * Views Field handler to show an author mapped to a drupal user if
 * possible. It also shows the committer if it is different.
 */
class versioncontrol_handler_field_operation_attribution extends versioncontrol_handler_field_operation_person {

  /**
   * An array to store {users}.name's keyed by uid for all the results.
   * This is needed in order to get names for author_uid and committer_uid at the same time.
   */
  public $usernames = array();

  function pre_render($values) {
    parent::pre_render($values);
    // Ensure the author alias is set.
    $this->aliases['author'] = $this->field_alias;

    // Get all uids taking care of repeated one.
    $uids = array();
    foreach ($values as $result) {
      $uids[$result->{$this->aliases['author_uid']}] = TRUE;
      $uids[$result->{$this->aliases['committer_uid']}] = TRUE;
    }
    $uids = array_keys($uids);

    if (count($uids) > 0) {
      $placeholders = implode(',', array_fill(0, count($uids), '%d'));
      $result = db_query("SELECT u.uid, u.name FROM {users} u WHERE u.uid IN ( $placeholders )", $uids);
      while ($row = db_fetch_object($result)) {
        $this->usernames[$row->uid] = $row->name;
      }
    }
  }

  function render($values) {
    $author_uid = $values->{$this->aliases['author_uid']};
    $committer_uid = $values->{$this->aliases['committer_uid']};

    // Render the author.
    $this->aliases['person_uid'] = $this->aliases['author_uid'];
    $this->field_alias = $this->aliases['author'];
    $this->aliases['person_username'] = 'author_username';
    $values->{$this->aliases['person_username']} = $this->usernames[$author_uid];
    $author = parent::render($values);

    // Show only author if they are equal.
    if ($author_uid == $committer_uid) {
      return t('by !author', array('!author' => $author));
    }

    // Show both if they are different.
    $this->aliases['person_uid'] = $this->aliases['committer_uid'];
    $this->field_alias = $this->aliases['committer'];
    $this->aliases['person_username'] = 'committer_username';
    $values->{$this->aliases['person_username']} = $this->usernames[$committer_uid];
    $committer = parent::render($values);
    if (empty($this->options['plain_text_output'])) {
      return t('<span class="authored-by">authored by !author</span>, <span class="committed-by">committed by !committer</span>', array(
        '!author' => $author, '!committer' => $committer));
    }
    return t('authored by !author, committed by !committer', array(
      '!author' => $author, '!committer' => $committer));
  }

}
