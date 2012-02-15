<?php
// $Id$
/**
 * @file
 * Contains the versioncontrol operation RSS row style plugin.
 */

/**
 * Plugin which render the individual_commit_view on the resulting
 * object and formats it as an RSS item.
 */
class versioncontrol_plugin_row_operation_rss extends views_plugin_row {
  // Basic properties that let the row style follow relationships.
  public $base_table = 'versioncontrol_operations';
  public $base_field = 'vc_op_id';

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $view_fields = array('none' => t('<None>'));
    foreach ($this->display->handler->get_handlers('field') as $field => $handler) {
      $view_fields[$field] = $handler->ui_name();
    }

    $form['title_field'] = array(
      '#type' => 'select',
      '#title' => t('Title field'),
      '#description' => t('The field that is going to be used as the RSS item title for each row.'),
      '#options' => $view_fields,
      '#default_value' => $this->options['title_field'],
    );
    $form['link_field'] = array(
      '#type' => 'select',
      '#title' => t('Link field'),
      '#description' => t('The field that is going to be used as the RSS item link for each row. This must be a drupal relative path.'),
      '#options' => $view_fields,
      '#default_value' => $this->options['link_field'],
    );
    $form['description_field'] = array(
      '#type' => 'select',
      '#title' => t('Description field'),
      '#description' => t('The field that is going to be used as the RSS item description for each row.'),
      '#options' => $view_fields,
      '#default_value' => $this->options['description_field'],
    );
    $form['creator_field'] = array(
      '#type' => 'select',
      '#title' => t('Creator field'),
      '#description' => t('The field that is going to be used as the RSS item creator for each row.'),
      '#options' => $view_fields,
      '#default_value' => $this->options['creator_field'],
    );
    $form['date_field'] = array(
      '#type' => 'select',
      '#title' => t('Publication date field'),
      '#description' => t('The field that is going to be used as the RSS item pubDate for each row. It need to be in RFC 2822 format.'),
      '#options' => $view_fields,
      '#default_value' => $this->options['date_field'],
    );
  }

  function validate() {
    $errors = parent::validate();
    $required_options = array('title_field', 'link_field', 'description_field', 'creator_field', 'date_field');
    foreach ($required_options as $required_option) {
      if (empty($this->options[$required_option]) || $this->options[$required_option] == 'none') {
        $errors[] = t('Row style plugin requires specifying which views fields to use for RSS item.');
        break;
      }
    }
    return $errors;
  }

  function render($row) {
    global $base_url;
    static $row_index;
    if (!isset($row_index)) {
      $row_index = 0;
    }
    $vc_op_id  = $row->{$this->field_alias};

    $item = new stdClass();
    $item->title = $this->get_field($row_index, $this->options['title_field']);
    $item->link = url($this->get_field($row_index, $this->options['link_field']), array('absolute' => TRUE));
    $item->description = $this->get_field($row_index, $this->options['description_field']);
    $item->elements = array(
      array('key' => 'pubDate', 'value' => $this->get_field($row_index, $this->options['date_field'])),
      array(
        'key' => 'dc:creator',
        'value' => $this->get_field($row_index, $this->options['creator_field']),
        'namespace' => array('xmlns:dc' => 'http://purl.org/dc/elements/1.1/'),
      ),
      array(
        'key' => 'guid',
        'value' => t('VCS Operation !vc_op_id at !base_url', array('!vc_op_id' => $vc_op_id, '!base_url' => $base_url)),
        'attributes' => array('isPermaLink' => 'false')
      ),
    );

    $row_index++;

    foreach ($item->elements as $element) {
      if (isset($element['namespace'])) {
        $this->view->style_plugin->namespaces = array_merge($this->view->style_plugin->namespaces, $element['namespace']);
      }
    }

    return theme($this->theme_functions(), $this->view, $this->options, $item, $this->field_alias);
  }

  /**
   * Convinience method  to retrieve field output.
   *
   * @param $index
   *   The index count of the row as expected by views_plugin_style::get_field().
   * @param $field_id
   *   The ID assigned to the required field in the display.
   */
  function get_field($index, $field_id) {
    if (empty($this->view->style_plugin) || !is_object($this->view->style_plugin) || empty($field_id)) {
      return '';
    }
    return $this->view->style_plugin->get_field($index, $field_id);
  }

}
