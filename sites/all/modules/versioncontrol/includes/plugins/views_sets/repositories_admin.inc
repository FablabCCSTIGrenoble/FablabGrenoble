<?php

$plugin = array (
  'title' => t('Administrative repository listing'),
  'description' => t('An administrative list of repositories, used in the VCAPI-provided repository management UI at !link', array('!link' => l('repository management UI', 'admin/content/versioncontrol-repositories'))),
  'handler' => array(
    'class' => 'VersioncontrolViewsSetRepositoriesAdmin',
  ),
);