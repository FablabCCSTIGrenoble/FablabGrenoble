<?php

/**
 * Implementation of hook_fieldgroup_default_groups().
 */
function fabmoments_fieldgroup_default_groups() {
  $groups = array();

  // Exported group: group_steps
  $groups['fabmoment-group_steps'] = array(
    'group_type' => 'multigroup',
    'type_name' => 'fabmoment',
    'group_name' => 'group_steps',
    'parent' => '',
    'label' => 'Steps',
    'settings' => array(
      'form' => array(
        'style' => 'fieldset',
        'description' => '',
      ),
      'display' => array(
        'description' => '',
        'teaser' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'full' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        '4' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'label' => NULL,
      ),
      'multigroup' => array(
        'multiple-columns' => 0,
        'required' => 0,
        'multiple' => 1,
        'labels' => array(
          '0' => '',
          '1' => '',
          '2' => '',
          '3' => '',
          '4' => '',
          '5' => '',
          '6' => '',
          '7' => '',
          '8' => '',
          '9' => '',
        ),
      ),
    ),
    'weight' => '0',
    'fields' => array(
      '0' => 'field_step_instruction',
      '1' => 'field_step_picture',
    ),
    'depth' => 0,
    'parents' => array(
      '0' => '',
    ),
  );

  // Translatables
  // Included for use with string extractors like potx.
  t('Steps');

  return $groups;
}
