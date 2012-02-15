<?php
/**
 * @file
 * Repo class
 */

/**
 * Contain fundamental information about the repository.
 */
abstract class VersioncontrolRepository implements VersioncontrolEntityInterface, Serializable {
  protected $_id = 'repo_id';

  /**
   * db identifier
   *
   * @var    int
   */
  public $repo_id;

  /**
   * repository name inside drupal
   *
   * @var    string
   */
  public $name;

  /**
   * VCS string identifier
   *
   * @var    string
   */
  public $vcs;

  /**
   * where it is
   *
   * @var    string
   */
  public $root;

  /**
   * how ot authenticate
   *
   * @var    string
   */
  public $authorization_method = 'versioncontrol_admin';

  /**
   * The method that this repository will use to update the operations table.
   *
   * This should correspond to constants provided by the backend provider.
   *
   * @var    integer
   */
  public $update_method = 0;

  /**
   * The Unix timestamp when this repository was last updated.
   *
   * @var    integer
   */
  public $updated = 0;

  /**
   * Repository lock. Repositories are locked when running a parse job to ensure
   * duplicate data does not enter the database.
   *
   * Zero indicates an unlocked repository; any nonzero is a timestamp
   * the time of the last lock.
   *
   * @var integer
   */
  public $locked = 0;

  /**
   * An array of additional per-repository settings, mostly populated by
   * third-party modules. It is serialized on DB.
   */
  public $data = array();

  /**
   * The backend associated with this repository
   *
   * @var VersioncontrolBackend
   */
  protected $backend;

  protected $built = FALSE;

  /**
   * An array describing the plugins that will be used for this repository.
   *
   * The current plugin types(array keys) are:
   * - author_mapper
   * - committer_mapper
   * - webviewer_url_handler
   * - repomgr
   * - auth_handler
   *
   * @var array
   */
  public $plugins = array();

  /**
   * An array of plugin instances (instanciated plugin objects).
   *
   * @var array
   */
  protected $pluginInstances = array();

  protected $defaultCrudOptions = array(
    'update' => array('nested' => TRUE),
    'insert' => array('nested' => TRUE),
    'delete' => array('purge bypass' => TRUE),
  );

  public function __construct($backend = NULL) {
    if ($backend instanceof VersioncontrolBackend) {
      $this->backend = $backend;
    }
    else if (variable_get('versioncontrol_single_backend_mode', FALSE)) {
      $backends = versioncontrol_get_backends();
      $this->backend = reset($backends);
    }
  }

  public function getBackend() {
    return $this->backend;
  }

  /**
   * Convenience method to set the repository lock to a specific value.
   */
  public function updateLock($timestamp = NULL) {
    if (is_null($timestamp)) {
      $timestamp = time();
    }
    $this->locked = $timestamp;
  }

  /**
   * Pseudo-constructor method; call this method with an associative array or
   * stdClass object containing properties to be assigned to this object.
   *
   * @param array $args
   */
  public function build($args = array()) {
    // If this object has already been built, bail out.
    if ($this->built == TRUE) {
      return FALSE;
    }

    foreach ($args as $prop => $value) {
      $this->$prop = $value;
    }
    if (!empty($this->data) && is_string($this->data)) {
      $this->data = unserialize($this->data);
    }
    if (!empty($this->plugins) && is_string($this->plugins)) {
      $this->plugins = unserialize($this->plugins);
    }
    $this->built = TRUE;
  }


  /**
   * Perform a log fetch, synchronizing VCAPI database data with the current
   * state of the repository.
   *
   * FIXME - this should be an abstract method, but until we can make VersioncontrolRepository abstract again, we'll have to live with the fatal error.
   *
   */
  public function fetchLogs() {
    throw new Exception('Cannot perform a log fetch using base VersioncontrolRepository; your loaded repository object must be backend-specific.', E_ERROR);
  }

  /**
   * Perform a full history synchronization, but first purge all existing
   * repository data so that the sync job starts from scratch.
   *
   * This method triggers a special set of hooks so that projects which have
   * data dependencies on the serial ids of versioncontrol entities can properly
   * recover from the purge & rebuild.
   *
   * // FIXME this must be refactored so that hook invocations occur on the same
   *    side of queueing as the history sync.
   */
  public function reSyncFromScratch($bypass = TRUE) {
    module_invoke_all('versioncontrol_repository_pre_resync', $this, $bypass);

    $this->purgeData($bypass);
    $this->fetchLogs();

    module_invoke_all('versioncontrol_repository_post_resync', $this, $bypass);

    // TODO ensure all controller caches are cleared
  }

  /**
   * Title callback for repository arrays.
   */
  public function titleCallback() {
    return check_plain($repository->name);
  }

  /**
   * Load known branches in a repository from the database as an array of
   * VersioncontrolBranch-descended objects.
   *
   * @param array $ids
   *   An array of branch ids. If given, only branches matching these ids will
   *   be returned.
   * @param array $conditions
   *   An associative array of additional conditions. These will be passed to
   *   the entity controller and composed into the query. The array should be
   *   key/value pairs with the field name as key, and desired field value as
   *   value. The value may also be an array, in which case the IN operator is
   *   used. For more complex requirements, FIXME finish!
   *   @see VersioncontrolEntityController::buildQuery() .
   *
   * @return
   *   An associative array of label objects, keyed on their
   */
  public function loadBranches($ids = array(), $conditions = array(), $options = array()) {
    $conditions['repo_id'] = $this->repo_id;
    return $this->backend->loadEntities('branch', $ids, $conditions, $options);
  }

  /**
   * Load known tags in a repository from the database as an array of
   * VersioncontrolTag-descended objects.
   *
   * @param array $ids
   *   An array of tag ids. If given, only tags matching these ids will be
   *   returned.
   * @param array $conditions
   *   An associative array of additional conditions. These will be passed to
   *   the entity controller and composed into the query. The array should be
   *   key/value pairs with the field name as key, and desired field value as
   *   value. The value may also be an array, in which case the IN operator is
   *   used. For more complex requirements, FIXME finish!
   *   @see VersioncontrolEntityController::buildQuery() .
   *
   * @return
   *   An associative array of label objects, keyed on their
   */
  public function loadTags($ids = array(), $conditions = array(), $options = array()) {
    $conditions['repo_id'] = $this->repo_id;
    return $this->backend->loadEntities('tag', $ids, $conditions, $options);
  }

  public function loadCommits($ids = array(), $conditions = array(), $options = array()) {
    $conditions['type'] = VERSIONCONTROL_OPERATION_COMMIT;
    $conditions['repo_id'] = $this->repo_id;
    return $this->backend->loadEntities('operation', $ids, $conditions, $options);
  }

  /**
   * Return TRUE if the account is authorized to commit in the actual
   * repository, or FALSE otherwise. Only call this function on existing
   * accounts or uid 0, the return value for all other
   * uid/repository combinations is undefined.
   *
   * FIXME deprecate this in favour of a plugin implementation.
   *
   * @param $uid
   *   The user id of the checked account.
   */
  public function isAccountAuthorized($uid) {
    if (!$uid) {
      return FALSE;
    }
    $approved = array();

    foreach (module_implements('versioncontrol_is_account_authorized') as $module) {
      $function = $module .'_versioncontrol_is_account_authorized';

      // If at least one hook_versioncontrol_is_account_authorized()
      // returns FALSE, the account is assumed not to be approved.
      if ($function($this, $uid) === FALSE) {
        return FALSE;
      }
    }
    return TRUE;
  }

  public function save($options = array()) {
    return empty($this->repo_id) ? $this->insert($options) : $this->update($options);
  }

  /**
   * Update a repository in the database, and invoke the necessary hooks.
   *
   * The 'repo_id' and 'vcs' properties of the repository object must stay
   * the same as the ones given on repository creation,
   * whereas all other values may change.
   */
  public function update($options = array()) {
    if (empty($this->repo_id)) {
      // This is supposed to be an existing repository, but has no repo_id.
      throw new Exception('Attempted to update a Versioncontrol repository which has not yet been inserted in the database.', E_ERROR);
    }

    // Append default options.
    $options += $this->defaultCrudOptions['update'];

    drupal_write_record('versioncontrol_repositories', $this, 'repo_id');

    $this->backendUpdate($options);

    // Everything's done, let the world know about it!
    module_invoke_all('versioncontrol_repository_entity_update', $this);
    return $this;
  }

  protected function backendUpdate($options) {}

  /**
   * Insert a repository into the database, and call the necessary hooks.
   *
   * @return
   *   The finalized repository array, including the 'repo_id' element.
   */
  public function insert($options = array()) {
    if (!empty($this->repo_id)) {
      // This is supposed to be a new repository, but has a repo_id already.
      throw new Exception('Attempted to insert a Versioncontrol repository which is already present in the database.', E_ERROR);
    }

    // Append default options.
    $options += $this->defaultCrudOptions['insert'];

    // drupal_write_record() will fill the $repo_id property on $this.
    drupal_write_record('versioncontrol_repositories', $this);

    $this->backendInsert($options);

    // Everything's done, let the world know about it!
    module_invoke_all('versioncontrol_repository_entity_insert', $this);
    return $this;
  }

  protected function backendInsert($options) {}

  /**
   * Delete a repository from the database, and call the necessary hooks.
   * Together with the repository, all associated commits are deleted as
   * well.
   */
  public function delete($options = array()) {
    // Append default options.
    $options += $this->defaultCrudOptions['delete'];

    // Delete all contained data.
    $this->purgeData($options['purge bypass']);

    db_delete('versioncontrol_repositories')
      ->condition('repo_id', $this->repo_id)
      ->execute();

    $this->backendDelete($options);

    module_invoke_all('versioncontrol_entity_repository_delete', $this);
  }

  /**
   * Purge all parsed log data from this repository. Optionally bypass the API
   * to go MUCH faster.
   *
   * @param bool $bypass
   *   Whether or not to bypass the API and perform all operations with a small
   *   number of large queries. Skips individual hook notifications, but fires
   *   its own hook and is FAR more efficient than running deletes
   *   entity-by-entity.
   */
  public function purgeData($bypass = TRUE) {
    if (empty($bypass)) {
      foreach ($this->loadBranches() as $branch) {
        $branch->delete();
      }
      foreach ($this->loadTags() as $tag) {
        $tag->delete();
      }
      foreach ($this->loadCommits() as $commit) {
        $commit->delete();
      }
    }
    else {
      $label_ids = db_select('versioncontrol_labels', 'vl')
        ->fields('vl', array('label_id'))
        ->condition('vl.repo_id', $this->repo_id)
        ->execute()->fetchAll(PDO::FETCH_COLUMN);

      if (!empty($label_ids)) {
        db_delete('versioncontrol_operation_labels')
          ->condition('label_id', $label_ids)
          ->execute();
      }

      db_delete('versioncontrol_operations')
        ->condition('repo_id', $this->repo_id)
        ->execute();

      db_delete('versioncontrol_labels')
        ->condition('repo_id', $this->repo_id)
        ->execute();

      db_delete('versioncontrol_item_revisions')
        ->condition('repo_id', $this->repo_id)
        ->execute();

      module_invoke_all('versioncontrol_repository_bypassing_purge', $this);
    }
  }

  protected function backendDelete($options) {}

  /**
   * Convinience method to call backend analogue.
   *
   * @param $revision
   *   The unformatted revision, as given in $operation->revision
   *   or $item->revision (or the respective table columns for those values).
   * @param $format
   *   Either 'full' for the original version, or 'short' for a more compact form.
   *   If the revision identifier doesn't need to be shortened, the results can
   *   be the same for both versions.
   */
  public function formatRevisionIdentifier($revision, $format = 'full') {
    return $this->backend->formatRevisionIdentifier($revision, $format);
  }

  /**
   * Convinience method to retrieve url handler.
   */
  public function getUrlHandler() {
    if (!isset($this->pluginInstances['webviewer_url_handler'])) {
      $plugin = $this->getPlugin('webviewer_url_handler', 'webviewer_url_handlers');
      $class_name = ctools_plugin_get_class($plugin, 'handler');
      if (!class_exists($class_name)) {
        throw new Exception("Plugin '{$this->plugins['webviewer_url_handler']}' of type 'webviewer_url_handlers' does not contain a valid class name in handler slot 'handler'", E_WARNING);
        return FALSE;
      }
      if (isset($this->data['webviewer_base_url']) && !empty($this->data['webviewer_base_url'])) {
        $webviewer_base_url = $this->data['webviewer_base_url'];
      }
      else {
        $variable = 'versioncontrol_repository_' . $this->backend->type . '_base_url_' . $plugin['name'];
        $webviewer_base_url = variable_get($variable, '');
      }
      $this->pluginInstances['webviewer_url_handler'] = new $class_name(
        $this, $webviewer_base_url, $plugin['url_templates']
      );
    }
    return $this->pluginInstances['webviewer_url_handler'];
  }

  /**
   * Get a ctools plugin based on plugin slot passed.
   */
  protected function getPlugin($plugin_slot, $plugin_type) {
    ctools_include('plugins');

    if (empty($this->plugins[$plugin_slot])) {
      // handle special case for two slots using the same plugin type
      if ($plugin_slot == 'committer_mapper' || $plugin_slot == 'author_mapper') {
        $variable = 'versioncontrol_repository_plugin_default_user_mapping_methods';
      }
      else {
        $variable = 'versioncontrol_repository_plugin_default_' . $plugin_slot;
      }
      $plugin_name = variable_get($variable, '');
    }
    else {
      $plugin_name = $this->plugins[$plugin_slot];
    }

    $plugin = ctools_get_plugins('versioncontrol', $plugin_type, $plugin_name);
    if (!is_array($plugin)) {
      throw new Exception("Attempted to get a plugin of type '$plugin_type' named '$plugin_name', but no such plugin could be found.", E_WARNING);
      return FALSE;
    }

    return $plugin;
  }

  /**
   * Get an instantiated plugin object based on a requested plugin slot, and the
   * plugin this repository object has assigned to that slot.
   *
   * Internal function - other methods should provide a nicer public-facing
   * interface. This method exists primarily to reduce code duplication involved
   * in ensuring error handling and sound loading of the plugin.
   */
  protected function getPluginClass($plugin_slot, $plugin_type, $class_type) {
    $plugin = $this->getPlugin($plugin_slot, $plugin_type);

    $class_name = ctools_plugin_get_class($plugin, $class_type);
    if (!class_exists($class_name)) {
      throw new Exception("Plugin '$plugin_slot' of type '$plugin_type' does not contain a valid class name in handler slot '$class_type'", E_WARNING);
      return FALSE;
    }

    $plugin_object = new $class_name();
    $this->getBackend()->verifyPluginInterface($this, $plugin_slot, $plugin_object);
    return $plugin_object;
  }

  public function getAuthHandler() {
    if (!isset($this->pluginInstances['auth_handler'])) {
      $this->pluginInstances['auth_handler'] = $this->getPluginClass('auth_handler', 'vcs_auth', 'handler');
      $this->pluginInstances['auth_handler']->setRepository($this);
    }
    return $this->pluginInstances['auth_handler'];
  }

  public function getAuthorMapper() {
    if (!isset($this->pluginInstances['author_mapper'])) {
      $this->pluginInstances['author_mapper'] = $this->getPluginClass('author_mapper', 'user_mapping_methods', 'mapper');
    }
    return $this->pluginInstances['author_mapper'];
  }

  public function getCommitterMapper() {
    if (!isset($this->pluginInstances['committer_mapper'])) {
      $this->pluginInstances['committer_mapper'] = $this->getPluginClass('committer_mapper', 'user_mapping_methods', 'mapper');
    }

    return $this->pluginInstances['committer_mapper'];
  }

  public function getRepositoryManager() {
    if (!isset($this->pluginInstances['repomgr'])) {
      $this->pluginInstances['repomgr'] = $this->getPluginClass('repomgr', 'repomgr', 'worker');
      $this->pluginInstances['repomgr']->setRepository($this);
    }

    return $this->pluginInstances['repomgr'];
  }

  /**
   * Fulfills Serializable::serialize() interface.
   *
   * @return string
   */
  public function serialize() {
    $refl = new ReflectionObject($this);
    // Get all properties, except static ones.
    $props = $refl->getProperties(ReflectionProperty::IS_PRIVATE | ReflectionProperty::IS_PROTECTED | ReflectionProperty::IS_PUBLIC );

    $ser = array();
    foreach ($props as $prop) {
      if (in_array($prop->name, array('backend', 'pluginInstances'))) {
        // serializing the backend is too verbose; serializing pluginInstances
        // could get us into trouble with autoload before D7.
        continue;
      }
      $ser[$prop->name] = $this->{$prop->name};
    }
    return serialize($ser);
  }

  /**
   * Fulfills Serializable::unserialize() interface.
   *
   * @param string $string_rep
   */
  public function unserialize($string_rep) {
    foreach (unserialize($string_rep) as $prop => $val) {
      $this->$prop = $val;
    }
    // And add the backend, which was stripped out.
    $this->backend = versioncontrol_get_backends($this->vcs);
  }
}
