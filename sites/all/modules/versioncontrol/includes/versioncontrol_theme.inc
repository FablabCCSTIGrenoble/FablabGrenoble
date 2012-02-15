<?php
/**
 * @file
 * Theme functions for Version Control API.
 */

/**
 * Return a rendered visual diffstat.
 */
function theme_versioncontrol_diffstat($places, $result) {
  $output = '';
  drupal_add_css(drupal_get_path('module', 'versioncontrol') . '/versioncontrol.css', 'module');
  for ($i = 0; $i++, $i <= $places;) {
    if ($result['add'] != 0) {
      $output .= '<span class="plus">+</span>';
      $result['add']--;
      continue;
    }
    if ($result['remove'] != 0) {
      $output .= '<span class="minus">-</span>';
      $result['remove']--;
      continue;
    }
    $output .= '<span class="no-op"> </span>';
  }
  return '<span class="versioncontrol-diffstat clear-block">'. $output .'</span>';
}

/**
 * Generate an RSS item element for one versioncontrol operation.
 */
function theme_versioncontrol_view_row_operation_rss($view, $options, $row, $field_alias) {
  $output = '<item>';
  $output .= '<title>' . $row->title . '</title>';
  $output .= '<link>' . $row->link . '</link>';
  $output .= '<description>' . check_plain($row->description) . '</description>';
  $output .= empty($row->elements) ? '' : format_xml_elements($row->elements);
  $output .= '</item>';
  return $output;
}
