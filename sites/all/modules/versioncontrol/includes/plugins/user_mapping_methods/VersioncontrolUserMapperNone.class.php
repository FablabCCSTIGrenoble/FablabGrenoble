<?php
//$Id

/**
 * Plugin that do not map.
 */
class VersioncontrolUserMapperNone implements VersioncontrolUserMapperInterface {
  public function mapAuthor(VersioncontrolOperation $commit) {
    return FALSE;
  }

  public function mapCommitter(VersioncontrolOperation $commit) {
    return FALSE;
  }
}
