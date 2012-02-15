<?php

/**
 * Views filter handler that pulls the formatted name from the backend object.
 *
 * This is purely a useability enhancement.
 */
class versioncontrol_handler_filter_repo_backend extends views_handler_filter_in_operator {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('VCS Backends');
      $types = versioncontrol_get_backends();
      foreach ($types as $type => $backend) {
        $options[$type] = t($backend->name);
      }
      $this->value_options = $options;
    }
  }
}
