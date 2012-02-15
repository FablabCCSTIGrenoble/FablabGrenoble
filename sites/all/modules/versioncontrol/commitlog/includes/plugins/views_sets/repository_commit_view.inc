<?php

$plugin = array (
  'title' => t('Single repository commit log'),
  'description' => t('A log of all commits made to a single repository.'),
  'handler' => array(
    'class' => 'VersioncontrolViewsSetRepositoryCommitView',
  ),
);