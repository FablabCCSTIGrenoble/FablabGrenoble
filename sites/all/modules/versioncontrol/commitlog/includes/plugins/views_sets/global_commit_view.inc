<?php

$plugin = array (
  'title' => t('Global commit log'),
  'description' => t('A log of all commits made in all repositories known to the system. Note that the backend-specific versions of this view will be used only if a single backend is enabled.'),
  'handler' => array(
    'class' => 'VersioncontrolViewsSetGlobalCommitView',
  ),
);