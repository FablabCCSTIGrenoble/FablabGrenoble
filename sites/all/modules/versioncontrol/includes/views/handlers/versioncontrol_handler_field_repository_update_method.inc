<?php

/**
 * Views Field handler to convert the numeric type field into text
 *
 */
class versioncontrol_handler_field_repository_update_method extends views_handler_field {

  /**
   * Called to add the field to a query.
   */
  function query() {
    $this->ensure_my_table();
    $this->field_alias = $this->query->add_field($this->table_alias, 'update_method');
    $this->backend_alias = $this->query->add_field($this->table_alias, 'vcs');
    $this->repo_id_alias = $this->query->add_field($this->table_alias, 'repo_id');
    $this->add_additional_fields();
  }

  function render($values) {
    $update_methods = versioncontrol_get_backends($values->{$this->backend_alias})->update_methods;
    return check_plain($update_methods[$values->{$this->field_alias}]);
  }
}
