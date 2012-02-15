<?php

/**
 * Plugin to do very simple mapping of email to Drupal users.
 *
 * Assumes $commit->author and $commit->committer are valid email addresses;
 * does no checking to ensure they're good.
 */
class VersioncontrolUserMapperSimpleMail implements VersioncontrolUserMapperInterface {
  public function mapAuthor(VersioncontrolOperation $commit) {
    return $this->map($commit->author);
  }

  public function mapCommitter(VersioncontrolOperation $commit) {
    return $this->map($commit->committer);
  }

  public function map($email) {
    $uid = db_result(db_query("SELECT uid FROM {users} WHERE mail = '%s'", $email));
    return empty($uid) ? FALSE : $uid;
  }
}
