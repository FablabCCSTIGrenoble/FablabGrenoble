<?php

/**
 * Views Field handler to provide repository manual log fetching links.
 */
class versioncontrol_handler_field_repository_manual_fetch extends views_handler_field {

  /**
   * Called to add the field to a query.
   */
  function query() {
    $this->ensure_my_table();
    // Force views to load the repo_id field when this field is added so that
    // we can appropriately build our links.
    $this->repo_alias = $this->query->add_field($this->table_alias, 'repo_id');
    $this->locked_alias = $this->query->add_field($this->table_alias, 'locked');
    $this->add_additional_fields();
  }

  function render($values) {
    $locked = $values->{$this->locked_alias};
    if ($locked == 0) {
      $link = array(
        'title' => t('Fetch logs'),
        'href' => 'admin/content/versioncontrol-repositories/fetch/' . $values->{$this->repo_alias},
      );
      return theme('links', array($link));
    }
    else {
      $link = array(
        'title' => t('Break lock'),
        'href' => 'admin/content/versioncontrol-repositories/clearlock/' . $values->{$this->repo_alias},
      );
      $output = theme('links', array($link));
      return  t('Locked since @date.', array('@date' => format_date($locked, 'small'))) . "$output";
    }
  }
}
