<?php
/**
 * @file
 * Repository Url handler class.
 */

/**
 * Base class for URL handlers.
 */
class VersioncontrolRepositoryUrlHandler implements VersioncontrolWebviewerUrlHandlerInterface {

  /**
   * Repository where this urls belongs.
   *
   * @var VersioncontrolRepository
   */
  public $repository;

  /**
   * The first part of the URL that is going to be prepended to each URL
   * retrieved.
   *
   * @var string
   */
  public $baseUrl;

  /**
   * An array of template URLs for the web viewer.
   *
   * @var array
   */
  public $templateUrls;

  public function __construct($repository, $base_url, $template_urls) {
    $this->repository = $repository;
    $this->baseUrl = $base_url;
    $this->templateUrls = $template_urls;
  }

  public function getTemplateUrl($name) {
    if (!isset($this->templateUrls[$name]) || empty($this->templateUrls[$name])) {
      return '';
    }
    return strtr($this->templateUrls[$name], array('%base_url' => $this->baseUrl));
  }

  /**
   * Retrieve the URL of the repository viewer that displays the main
   * view of the repository.
   *
   * @return
   *   The repository view URL of the associated repository.
   */
  public function getRepositoryViewUrl() {
    return strtr($this->getTemplateUrl('repository_view'), array(
      '%repo_name' => $this->repository->name,
    ));
  }

  /**
   * Retrieve the URL of the repository viewer that displays the given commit
   * in the corresponding repository.
   *
   * @param $revision
   *   The revision on the commit operation whose view URL should be retrieved.
   *
   * @return
   *   The commit view URL corresponding to the given arguments.
   *   An empty string is returned if no commit view URL has been defined,
   *   or if the commit cannot be viewed for any reason.
   */
  public function getCommitViewUrl($revision) {
    if (empty($revision)) {
      return '';
    }
    return strtr($this->getTemplateUrl('commit_view'), array(
      '%repo_name' => $this->repository->name,
      '%revision' => $revision,
    ));
  }

  /**
   * Retrieve the URL of the repository viewer that displays the commit log
   * of the given item in the corresponding repository. If no such URL has been
   * specified by the user, the appropriate URL from the Commit Log module is
   * used as a fallback (if that module is enabled).
   *
   * @param $item
   *   The item whose log view URL should be retrieved.
   *
   * @return
   *   The item log view URL corresponding to the given arguments.
   *   An empty string is returned if no item log view URL has been defined
   *   (and if not even Commit Log is enabled), or if the item cannot be viewed
   *   for any reason.
   */
  public function getItemLogViewUrl($item) {
    $label = $item->getSelectedLabel();

    if (isset($label->type) && $label->type == VERSIONCONTROL_LABEL_BRANCH) {
      $current_branch = $label->name;
    }

    $placeholders = array(
      '%repo_name' => $this->repository->name,
      '%path'     => $item->path,
      '%revision' => $item->revision,
      '%branch'   => isset($current_branch) ? $current_branch : '',
    );

    if ($item->isFile()) {
      return strtr($this->getTemplateUrl('file_log_view'), $placeholders);
    }
    else { // directory
      return strtr($this->getTemplateUrl('directory_log_view'), $placeholders);
    }
  }

  /**
   * Retrieve the URL of the repository viewer that displays the contents of the
   * given item in the corresponding repository.
   *
   * @param $item
   *   The item whose view URL should be retrieved.
   *
   * @return
   *   The item view URL corresponding to the given arguments.
   *   An empty string is returned if no item view URL has been defined,
   *   or if the item cannot be viewed for any reason.
   */
  public function getItemViewUrl($item) {
    $label = $item->getSelectedLabel();

    if (isset($label->type) && $label->type == VERSIONCONTROL_LABEL_BRANCH) {
      $current_branch = $label->name;
    }
    $view_url = $item->isFile()
      ? $this->getTemplateUrl('file_view')
      : $this->getTemplateUrl('directory_view');

    return strtr($view_url, array(
      '%repo_name' => $this->repository->name,
      '%path'     => $item->path,
      '%revision' => $item->revision,
      '%branch'   => isset($current_branch) ? $current_branch : '',
    ));
  }

  /**
   * Retrieve the URL of the repository viewer that displays the diff between
   * two given files in the corresponding repository.
   *
   * @param $file_item_new
   *   The new version of the file that should be diffed.
   * @param $file_item_old
   *   The old version of the file that should be diffed.
   *
   * @return
   *   The diff URL corresponding to the given arguments.
   *   An empty string is returned if no diff URL has been defined,
   *   or if the two items cannot be diffed for any reason.
   */
  public function getDiffUrl($file_item_new, $file_item_old) {
    $label = $file_item_new->getSelectedLabel();

    if (isset($label->type) && $label->type == VERSIONCONTROL_LABEL_BRANCH) {
      $current_branch = $label->name;
    }
    return strtr($this->getTemplateUrl('diff'), array(
      '%repo_name' => $this->repository->name,
      '%path'         => $file_item_new->path,
      '%new_revision' => $file_item_new->revision,
      '%old_path'     => $file_item_old->path,
      '%old_revision' => $file_item_old->revision,
      '%branch'       => isset($current_branch) ? $current_branch : '',
    ));
  }

}
