<?php

/**
 * @file worker.inc
 *
 * Queue-processing worker-related functions.
 */

/**
 * Process a versioncontrol_repomgr queue job.
 *
 * @param type $data
 *   The array of job data to be processed.
 */
function _versioncontrol_repomgr_run_worker($data) {
  // Lots and LOTS of input validation; this is a really bad place to fail silently.
  if (empty($data['repository'])) {
    $str = "Repository management job payload contained no repository data.";
    watchdog('versioncontrol', $str, array(), WATCHDOG_ERROR);
    throw new Exception($str, E_ERROR);
  }
  else if ($data['repository'] instanceof VersioncontrolRepository) {
    // Full object was serialized, no loading required
    $repo = $data['repository'];
  }
  else {
    // Expect that a repo_id was sent and we load it out here
    $repo = versioncontrol_repository_load($data['repository']);
    if (!$repo instanceof VersioncontrolRepository) {
      $str = "Repository management job enqueued with repo_id '%repo_id', which failed to load a valid VersioncontrolRepository object.";
      $vars = array(
        '%repo_id' => $data['repository'],
      );
      watchdog('versioncontrol', $str, $vars, WATCHDOG_ERROR);
      throw new Exception(strtr($str, $vars), E_ERROR);
    }
  }

  $manager = $repo->getRepositoryManager();

  if (empty($data['operation'])) {
    watchdog('versioncontrol', "Repository management job was enqueued without any operations to run.", array(), WATCHDOG_ERROR);
    throw new Exception("Repository management job was enqueued without any operations to run.", E_ERROR);
  }

  $ops = is_string($data['operation']) ? array($data['operation'] => array()) : $data['operation'];

  // Loop through all ops and ensure all are valid before running any. This
  // helps preserve known state by preventing jobs are guaranteed to terminate
  // partway through from starting at all.
  foreach (array_keys($ops) as $op) {
    if (!method_exists($manager, $op)) {
      $vars = array(
        '%op' => $op,
        '%repo_id' => empty($repo->repo_id) ? '[NEW]' : $repo->repo_id,
        '%plugin' => get_class($manager),
      );
      watchdog('versioncontrol', "Operation '%op' was requested on repo_id %repo_id using plugin class %plugin, but no such method exists.", $vars, WATCHDOG_ERROR);
      throw new Exception(strtr("Operation '%op' was requested on repo_id %repo_id using plugin class %plugin, but no such method exists.", $vars), E_ERROR);
    }
  }

  // Finally ready to actually run the operations.
  foreach ($ops as $op => $args) {
    call_user_func_array(array($manager, $op), $args);
  }
}