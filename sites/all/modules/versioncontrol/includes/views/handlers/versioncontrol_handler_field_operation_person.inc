<?php

/**
 * Views Field handler to show an author or committer, mapped to a
 * drupal user if possible.
 */
class versioncontrol_handler_field_operation_person extends views_handler_field {

  function option_definition() {
    $options = parent::option_definition();
    $options['plain_text_output'] = array('default' => 0, 'translatable' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['plain_text_output'] = array(
      '#type' => 'checkbox',
      '#title' => t('Plain text output'),
      '#default_value' => $this->options['plain_text_output'],
      '#description' => t('Avoid markup and links on this field output.'),
    );
  }

  function render($values) {
    if (empty($values->{$this->aliases['person_uid']})) {
      return $values->{$this->field_alias};
    }
    else {
      $account = new stdClass();
      $account->uid = $values->{$this->aliases['person_uid']};
      // Show the value on the field if not mapped.
      if ($account->uid == 0) {
        $account->name = $values->{$this->field_alias};
      }
      else {
        if (empty($values->{$this->aliases['person_username']})) {
          return $values->{$this->field_alias};
        }
        else {
          $account->name = $values->{$this->aliases['person_username']};
        }
      }
      if (empty($this->options['plain_text_output'])) {
        return theme('username', $account);
      }
      return $account->name;
    }
  }

}
