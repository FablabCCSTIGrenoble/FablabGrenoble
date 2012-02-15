<?php

/**
 * Views Field handler to provide a combined output of lines added and deleted
 * from a file.
 */
class versioncontrol_handler_field_item_revision_lines_changed extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['line_changes_added'] = 'line_changes_added';
    $this->additional_fields['line_changes_removed'] = 'line_changes_removed';
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $added = $values->{$this->aliases['line_changes_added']};
    $removed = $values->{$this->aliases['line_changes_removed']};

    $additions = format_plural($added, '1 addition', '@count additions');
    $removals = format_plural($removed, '1 deletion', '@count deletions');

    return t('!addition & !removal', array('!addition' => $additions, '!removal' => $removals));
  }
}
