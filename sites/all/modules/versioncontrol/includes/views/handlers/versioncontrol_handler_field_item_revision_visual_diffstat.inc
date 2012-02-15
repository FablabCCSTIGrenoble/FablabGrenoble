<?php

/**
 * Views Field handler to provide visual diffstat representation of changes to
 * a specific file.
 */
class versioncontrol_handler_field_item_revision_visual_diffstat extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['line_changes_added'] = 'line_changes_added';
    $this->additional_fields['line_changes_removed'] = 'line_changes_removed';
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['places'] = array(
      '#type' => 'select',
      '#title' => t('Select how many places should be displayed'),
      '#options' => array(3 => 3, 5 => 5, 7 => 7, 9 => 9),
      '#default_value' => $this->options['places'],
    );
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $places = $this->options['places'];

    // Calculate total and actual percentages
    $added = $values->{$this->aliases['line_changes_added']};
    $removed = $values->{$this->aliases['line_changes_removed']};
    $total = $added + $removed;
    if ($total != 0) {
      $add_percentage = $added / $total;
      $remove_percentage = $removed / $total;
    }

    // Determine if we need to calculate adjusted percentages
    $adjust_perentages = ($total > $places) ? TRUE : FALSE;

    // Determine is deletion or addition ONLY
    $single_operation = ($added == 0 || $removed == 0) ? TRUE : FALSE;

    // If we are only showing additions or deletions, or we have less changes
    // to show than places, just display without using percentages.
    if ($single_operation || !$adjust_perentages) {
      $result = array('add' => $added, 'remove' => $removed);
    } else {
      // Adjust percentages so that if too close to 50%, they are rounded down
      $this->adjust_percentages($places, $add_percentage);
      $this->adjust_percentages($places, $remove_percentage);

      $add = round($add_percentage * $places);
      $remove = round($remove_percentage * $places);

      $result = array('add' => $add, 'remove' => $remove);
    }

    return theme('versioncontrol_diffstat', $places, $result);
  }

  function adjust_percentages($places, &$percentage) {
    // Calculate percentages, deviation and range
    $place_percentage = 1 / $places;
    $deviation = $place_percentage / 2;

    // Calculate center percentages for items close to 50%
    $lower = .5 - $deviation;
    $upper = .5 + $deviation;
    if ($percentage >= $lower && $percentage < $upper) {
      if ($percentage <= .5) {
        $percentage = $percentage - $deviation;
      }
      else {
        $percentage = $percentage + $deviation;
      }
      return;
    }

    // Caclulate edge percentages, for items close to 100%
    $lower =  $deviation;
    $upper =  1 - $deviation;
    if ($percentage >= $upper || $percentage < $lower) {
      $percentage = $percentage - $deviation;
      return;
    }
  }
}
