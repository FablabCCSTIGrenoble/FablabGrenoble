<?php

class VersioncontrolAuthHandlerFFA implements VersioncontrolAuthHandlerInterface {
  public function setRepository(VersioncontrolRepository $repository) {}

  public function authAccess($uid) {
    return TRUE;
  }

  public function authBranchCreate($uid, VersioncontrolBranch $branch) {
    return TRUE;
  }
  public function authBranchDelete($uid, VersioncontrolBranch $branch) {
    return TRUE;
  }
  public function authBranchUpdate($uid, VersioncontrolBranch $branch) {
    return TRUE;
  }
  public function authTagCreate($uid, VersioncontrolTag $tag) {
    return TRUE;
  }
  public function authTagDelete($uid, VersioncontrolTag $tag) {
    return TRUE;
  }
  public function authTagUpdate($uid, VersioncontrolTag $tag) {
    return TRUE;
  }

  public function getErrorMessages() {
    return array();
  }
}
