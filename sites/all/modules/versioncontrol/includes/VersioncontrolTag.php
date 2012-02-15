<?php
/**
 * @file
 * Repo Tag class
 */

/**
 * Represents a tag of code (not changing state)
 */
class VersioncontrolTag extends VersioncontrolEntity {
  protected $_id = 'label_id';

  /**
   * The tag identifier (a simple integer), used for unique identification of
   * this tag in the database.
   *
   * @var int
   */
  public $label_id;

  /**
   * The tag name.
   *
   * @var string
   */
  public $name;

  /**
   * Indicates this is a tag; for db interaction only.
   *
   * @var int
   */
  public $type = VERSIONCONTROL_LABEL_TAG;

  /**
   * @name VCS actions
   * for a single item (file or directory) in a commit, or for branches and tags.
   * either VERSIONCONTROL_ACTION_{ADDED,MODIFIED,MOVED,COPIED,MERGED,DELETED,
   * REPLACED,OTHER}
   *
   * @var array
   */
  public $action;

  /**
   * The database id of the repository with which this tag is associated.
   * @var int
   */
  public $repo_id;

  public function update($options = array()) {
    if (empty($this->label_id)) {
      // This is supposed to be an existing tag, but has no label_id.
      throw new Exception('Attempted to update a Versioncontrol branch which has not yet been inserted in the database.', E_ERROR);
    }

    // Append default options.
    $options += $this->defaultCrudOptions['update'];

    // make sure repo id is set for drupal_write_record()
    if (empty($this->repo_id)) {
      $this->repo_id = $this->repository->repo_id;
    }
    drupal_write_record('versioncontrol_labels', $this, 'label_id');

    // Let the backend take action.
    $this->backendUpdate($options);

    // Everything's done, invoke the hook.
    module_invoke_all('versioncontrol_entity_tag_update', $this);
    return $this;
  }

  public function insert($options = array()) {
    if (!empty($this->label_id)) {
      // This is supposed to be a new tag, but has a label_id already.
      throw new Exception('Attempted to insert a Versioncontrol branch which is already present in the database.', E_ERROR);
    }

    // Append default options.
    $options += $this->defaultCrudOptions['insert'];

    // make sure repo id is set for drupal_write_record()
    if (empty($this->repo_id)) {
      $this->repo_id = $this->repository->repo_id;
    }
    drupal_write_record('versioncontrol_labels', $this);

    $this->backendInsert($options);

    // Everything's done, invoke the hook.
    module_invoke_all('versioncontrol_entity_tag_insert', $this);
    return $this;
  }

  public function delete($options = array()) {
    // Append default options.
    $options += $this->defaultCrudOptions['delete'];

    db_delete('versioncontrol_operation_labels')
      ->condition('label_id', $this->label_id)
      ->execute();

    db_delete('versioncontrol_labels')
      ->condition('label_id', $this->label_id)
      ->execute();

    $this->backendDelete($options);

    module_invoke_all('versioncontrol_entity_tag_delete', $this);
  }
}
