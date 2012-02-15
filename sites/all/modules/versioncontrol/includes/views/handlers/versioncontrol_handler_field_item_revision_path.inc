<?php
class versioncontrol_handler_field_item_revision_path extends views_handler_field {
  public $backends = NULL;
  public $repos = array();
  public $itemRevisions = array();

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

  /**
   * Get an item revision.
   */
  function getItemRevision($vcs, $item_revision_id) {
    if (!isset($this->itemRevisions[$item_revision_id])) {
      $this->itemRevisions[$item_revision_id] = $this->backends[$vcs]->loadEntity('item', array($item_revision_id));
    }
    return $this->itemRevisions[$item_revision_id];
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['link_type'] = array('default' => 'view', 'translatable' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_type'] = array(
      '#type' => 'radios',
      '#title' => t('Where to link at the repository browser'),
      '#default_value' => $this->options['link_type'],
      '#options' => array(
        'view' => t('Item content'),
        'log' => t('Item log history'),
      ),
    );
  }

  function pre_render($values) {
    $revision_ids = array();
    foreach ($values as $result) {
      $vcs = $result->{$this->aliases['vcs']};
      $revision_ids[$vcs][] = $result->{$this->aliases['item_revision_id']};
    }
    $this->itemRevisions = array();
    foreach ($revision_ids as $vcs => $ids) {
      $this->itemRevisions += $this->backends[$vcs]->loadEntities('item', $ids);
    }
  }

  function render($values) {
    $path = $values->{$this->field_alias};
    $vcs = $values->{$this->aliases['vcs']};
    $repo_id = $values->{$this->aliases['repo_id']};
    $item_revision_id = $values->{$this->aliases['item_revision_id']};
    $repo = $this->getRepository($vcs, $repo_id);
    $url_handler = $repo->getUrlHandler();
    $item_revision = $this->getItemRevision($vcs, $item_revision_id);
    $link_type = !empty($this->options['link_type']) ? $this->options['link_type'] : 'view';
    switch ($link_type) {
      case 'log':
        $link = $url_handler->getItemLogViewUrl($item_revision);
      case 'view':
      default:
        $link = $url_handler->getItemViewUrl($item_revision);
    }
    if (!empty($link)) {
      return l(check_plain($path), $link);
    }
    return check_plain($path);
  }
}
