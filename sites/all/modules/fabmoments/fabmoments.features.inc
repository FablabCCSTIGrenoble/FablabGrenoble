<?php

/**
 * Implementation of hook_node_info().
 */
function fabmoments_node_info() {
  $items = array(
    'fabmoment' => array(
      'name' => t('FabMoment'),
      'module' => 'features',
      'description' => t('FabMoment describes how a thing gets made'),
      'has_title' => '1',
      'title_label' => t('Project name'),
      'has_body' => '1',
      'body_label' => t('Summary'),
      'min_word_count' => '0',
      'help' => '',
    ),
  );
  return $items;
}
