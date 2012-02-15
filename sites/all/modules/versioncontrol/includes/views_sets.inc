<?php

/**
 * @file
 *
 * Interfaces and base classes that define VCAPI's views set system.
 */

/**
 * Interface defining the behavior for the classes used by views set plugins.
 *
 * Views sets are wrappers around groups of views that VCAPI uses to ease the
 * process of determining the most appropriate view (either generic or
 * backend-specific) to use in a given situation.  A views set plugin represents
 * something that would, under most circumstances, be a single view, but because
 * vcapi has its various backends, it needs the group. Plugins implementing this
 * interface are responsible for the logic of that multiplexing.
 */
interface VersioncontrolViewsSetInterface {
  /**
   * Derive the appropriate view for this set based on the data in a
   * VersioncontrolEntity object.
   *
   * @param VersioncontrolEntity $entity
   *   The entity from which a view should be derived.
   */
  public function getViewNameByEntity(VersioncontrolEntityInterface $entity);

  /**
   * Retrieve the appropriate view based on an explicitly known backend.
   *
   * @param mixed $backend
   *   Either a VersioncontrolBackend object, or the short name that uniquely
   *   refers to a backend.
   */
  public function getViewNameByBackend($backend);

  public function getSetName();

  public function getDefaultViews($vcs = '');

  public function getBaseView();

  /**
   * \o/
   */
  public function setSet($data);
}

interface VersioncontrolViewsGlobalSetInterface extends VersioncontrolViewsSetInterface {
  /**
   * Retrieve the appropriate view, typically based on some environment data.
   *
   * Global views sets are, under most circumstances, sensitive only to whether
   * or not VCAPI is operating in single backend mode (as defined by the
   * global $conf variable). Thus, they tend not to need an argument to derive
   * the appropriate view.
   */
  public function getViewName();
}

/**
 * Base class for views sets; reduce a little code duplication.
 *
 */
abstract class VersioncontrolViewsSetBase implements VersioncontrolViewsSetInterface {
  protected $setName = '';

  protected $baseView = '';

  protected $setData = array();

  protected $defaultViews = array();

  /**
   * Grab stored set data and attach it as a property.
   */
  public function __construct() {
    // $this->setData = versioncontrol_get_views_set($this->setName);
  }

  public function getViewNameByEntity(VersioncontrolEntityInterface $entity) {
    return $this->getViewNameByBackend($entity->getBackend());
  }

  public function getViewNameByBackend($backend) {
    if ($backend instanceof VersioncontrolBackend) {
      $backend = $backend->type;
    }
    return empty($this->setData[$backend]) ? $this->baseView : $this->setData[$backend];
  }

  public function getBaseView() {
    return $this->baseView;
  }

  public function getDefaultViews($vcs = '') {
    if (empty($vcs)) {
      return $this->defaultViews;
    }

    return empty($this->defaultViews[$vcs]) ? FALSE : $this->defaultViews[$vcs];
  }

  public function getSetName() {
    return $this->setName;
  }

  public function setSet($data) {
    $this->setData = $data;
  }
}

/**
 * Helper abstract parent for global views sets. For most of them, the logic
 * defined here will be perfectly adequate. If the logic doesn't work, others
 * can still just implement the interface directly.
 */
abstract class VersioncontrolViewsSetGlobalBase extends VersioncontrolViewsSetBase implements VersioncontrolViewsGlobalSetInterface {

  protected $view;

  public function getViewNameByEntity(VersioncontrolEntityInterface $entity) {
    return $this->view;
  }

  public function getViewName() {
    return $this->view;
  }

  public function setSet($data) {
    parent::setSet($data);

    if (variable_get('versioncontrol_single_backend_mode', FALSE)) {
      $backends = versioncontrol_get_backends();
      $this->view = $this->getViewNameByBackend(reset($backends));
    }
    else {
      $this->view = $this->baseView;
    }
  }
}