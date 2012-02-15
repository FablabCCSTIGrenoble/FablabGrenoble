<?php

$plugin = array (
  'title' => t('Per-user commit log'),
  'description' => t('A log of all commits made by a specific user in all repositories known to the system. As with the global commit view, note that the backend-specific version will only be used if operating in single-backend mode.'),
  'handler' => array(
    'class' => 'VersioncontrolViewsSetUserCommitView',
  ),
);