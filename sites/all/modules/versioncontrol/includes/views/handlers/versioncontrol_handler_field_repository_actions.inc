<?php

/**
 * Views Field handler to provide repository action links.
 */
class versioncontrol_handler_field_repository_actions extends views_handler_field {

  /**
   * Called to add the field to a query.
   */
  function query() {
    $this->ensure_my_table();
    // Force views to load the repo_id field when this field is added so that
    // we can appropriately build our links.
    $this->field_alias = $this->query->add_field($this->table_alias, 'repo_id');
    $this->add_additional_fields();
  }

  function render($values) {
    $links = array(
      array(
        'title' => t('Edit'),
        'href' => 'admin/content/versioncontrol-repositories/edit/'. $values->{$this->field_alias},
      ),
      array(
        'title' => t('Delete'),
        'href' => 'admin/content/versioncontrol-repositories/delete/'. $values->{$this->field_alias},
      ),
    );
    return theme('links', $links);
  }
}
