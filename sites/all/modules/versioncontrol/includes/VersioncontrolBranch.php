<?php
/**
 * @file
 * Repo Branch class
 */

/**
 * Represents a repository branch.
 */
class VersioncontrolBranch extends VersioncontrolEntity {
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
   * Indicates this is a branch; for db interaction only.
   *
   * @var int
   */
  public $type = VERSIONCONTROL_LABEL_BRANCH;

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
   * The database id of the repository with which this branch is associated.
   * @var int
   */
  public $repo_id;

  protected $defaultCrudOptions = array(
    'update' => array('nested' => TRUE),
    'insert' => array('nested' => TRUE),
    'delete' => array('nested' => TRUE),
  );

  /**
   * Load commits from the database that are associated with this branch.
   *
   * @param array $ids
   * @param array $conditions
   * @param array $options
   */
  public function loadCommits($ids = array(), $conditions = array(), $options = array()) {
    $conditions['branches'] = array($this->label_id);
    return $this->backend->loadEntities('operation', $ids, $conditions, $options);
  }

  public function update($options = array()) {
    if (empty($this->label_id)) {
      // This is supposed to be an existing branch, but has no label_id.
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
    module_invoke_all('versioncontrol_entity_branch_update', $this);
    return $this;
  }

  public function insert($options = array()) {
    if (!empty($this->label_id)) {
      // This is supposed to be a new branch, but has a label_id already.
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
    module_invoke_all('versioncontrol_entity_branch_insert', $this);
    return $this;
  }

  /**
   * Delete this branch from the database, along with all related data.
   */
  public function delete($options = array()) {
    // Append default options.
    $options += $this->defaultCrudOptions['delete'];

    if (!empty($options['nested'])) {
      $this->deleteRelatedCommits($options);
    }

    db_delete('versioncontrol_operation_labels')
      ->condition('label_id', $this->label_id)
      ->execute();

    db_delete('versioncontrol_labels')
      ->condition('label_id', $this->label_id)
      ->execute();

    $this->backendDelete($options);

    module_invoke_all('versioncontrol_entity_branch_delete', $this);
  }

  protected function deleteRelatedCommits($options) {
    $commits = $this->loadCommits();
    foreach ($commits as $commit) {
      // Only delete the commit if this branch is the commit's only branch.
      $sole_branch = TRUE;
      foreach ($commit->labels as $label_id => $label) {
        if ($label_id != $this->label_id && $label->type != VERSIONCONTROL_LABEL_TAG) {
          $sole_branch = FALSE;
        }
      }

      // Either remove the commit entirely...
      if ($sole_branch) {
        $commit->delete();
      }
      // Or remove only the reference to this branch on the commit.
      else {
        unset($commit->labels[$this->label_id]);
      }
    }
  }
}
