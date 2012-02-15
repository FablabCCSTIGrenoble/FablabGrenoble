<?php

/**
 * Views Field handler to convert the numeric type field into text
 *
 */
class versioncontrol_handler_field_operation_type extends views_handler_field {

  function render($values) {
    $type = $values->{$this->field_alias};
    $types = versioncontrol_operation_types();
    if (isset($types[$type])) {
      return $types[$type];
    }
    return '';
  }
}
