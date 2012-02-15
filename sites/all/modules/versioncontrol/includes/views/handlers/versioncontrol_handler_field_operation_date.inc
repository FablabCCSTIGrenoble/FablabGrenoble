<?php
/**
 * A handler to provide date for operation with optional link to operation.
 */
class versioncontrol_handler_field_operation_date extends views_handler_field_date {
  function option_definition() {
    $options = parent::option_definition();

    $options['link'] = TRUE;

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['link'] = array(
      '#type' => 'checkbox',
      '#title' => t('Link this field to individual commit view'),
      '#default_value' => isset($this->options['link']) ? $this->options['link'] : TRUE,
    );
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $this->field_alias = $this->aliases['date_value'];
    if (!$this->options['link']) {
      return parent::render($values);
    }
    else {
      $date_field = parent::render($values);
      $link = sprintf('commitlog/commit/%d/%s', $values->{$this->aliases['repo_id']}, $values->{$this->aliases['revision']});
      return l($date_field, $link);
    }

  }
}
