<?php

/**
 * Version Control API field handler to output the right value of the
 * VersioncontrolOperation::revision.
 */
class versioncontrol_handler_field_operation_revision extends views_handler_field {
  public $backends = NULL;
  public $repos = array();

  function construct() {
    parent::construct();
    $this->backends = versioncontrol_get_backends();
  }

  /**
   * Get a repository of the @param $vcs type.
   */
  function getRepository($vcs, $repo_id) {
    if (!isset($this->repos[$repo_id])) {
      $this->repos[$repo_id] = $this->backends[$vcs]->loadEntity('repo', array($repo_id));
    }
    return $this->repos[$repo_id];
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['plain_text_output'] = array('default' => 0, 'translatable' => TRUE);
    $options['operation_format'] = array('default' => 'short', 'translatable' => TRUE);
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
    $form['operation_format'] = array(
      '#type' => 'radios',
      '#title' => t('Revision format'),
      '#default_value' => $this->options['operation_format'],
      '#options' => array('full' => t('Full'), 'short' => t('Short')),
      '#description' => t('The format mode the revision will use.'),
    );
  }

  function render($values) {
    $revision = $values->{$this->field_alias};
    $vcs = $values->{$this->aliases['vcs']};
    $repo_id = $values->{$this->aliases['repo_id']};
    $revision = $this->backends[$vcs]->formatRevisionIdentifier($revision, $this->options['operation_format']);
    $repo = $this->getRepository($vcs, $repo_id);
    if (!empty($this->options['plain_text_output'])) {
      return check_plain($revision);
    }
    $url_handler = $repo->getUrlHandler();
    $link = $url_handler->getCommitViewUrl($revision);
    if (!empty($link)) {
      return l(check_plain($revision), $link);
    }
    return check_plain($revision);
  }
}
