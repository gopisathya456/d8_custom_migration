<?php

/**
 * @file
 *   Code to convert D7 Node Reference fields to Entity Reference fields in D8.
 *
 * Problem being solved: There is no migration path for D7 References fields to D8 Entityreference fields.
 * This is an alternative to https://www.drupal.org/project/entityreference_migration, which requires that you convert the D7
 * database, and them migrate, which may introduce problems and may not be possible in all cases.
 *
 * NOTE THAT THIS CODE DEPENDS ON A PATCH, AND WILL NOT WORK WITHOUT IT:
 *   Patch to migrate D7 entityreference fields, which includes some code needed to migrate node reference field settings:
 *   - https://www.drupal.org/files/issues/field-upgrade_path_to_entity_reference_field_from_7.x-2611066-22-D8.1.x.patch
 */
use Drupal\migrate\Row;
use Drupal\migrate\Plugin\MigrateSourceInterface;
use Drupal\migrate\Plugin\MigrationInterface;

function custom_migration_node_reference(Row $row, MigrateSourceInterface $source, MigrationInterface $migration) {

  // List all the nodereference field names on this site.
  $nodereference_fields = array(
    'field_gallery',
    'field_gallery_images',
    'field_gallery_image_usage',
    'field_parent_id',
  );
  // List all the userreference field names on this site.
  $userreference_fields = array(

  );

  // Convert node_reference fields into entity_reference fields.
  if ($migration->id() == CUSTOM_MIGRATION_PREFIX . 'd7_field') {
    $process = $migration->getProcess();
    $process['type'][0]['map']['node_reference'] = 'entity_reference';
    $process['type'][0]['map']['user_reference'] = 'entity_reference';
    $migration->setProcess($process);
  }

  if ($migration->id() == CUSTOM_MIGRATION_PREFIX . 'd7_field_instance') {

    // Transform node_reference settings to the format expected by entity_reference.
    // Requires patch from https://www.drupal.org/files/issues/2611066-9.patch to make
    // field_settings available to the instance row object.
    if (in_array($row->getSourceProperty('type'), array('node_reference', 'user_reference'))) {
      if ($field_settings = @$row->getSourceProperty('field_settings')) {
        $settings = $row->getSourceProperty('instance_settings');
        $settings['handler'] = 'default:node';
        $settings['handler_settings']['target_bundles'] = [];
        $node_types = array_filter($field_settings['referenceable_types']);
        if (!empty($node_types)) {
          $settings['handler_settings']['target_bundles'] = $node_types;
        }
        $row->setSourceProperty('instance_settings', $settings);
      }
    }
  }

  if ($migration->id() == CUSTOM_MIGRATION_PREFIX + 'd7_field_instance_widget_settings') {
    // Map the changes in widget names. This does not update widget settings.
    // They have to be updated manually after migration.
    $widget_map = array(
      'options_select' => 'options_select',
      'options_buttons' => 'options_buttons',
      'node_reference_autocomplete' => 'entity_reference_autocomplete',
      'node_reference_url' => 'entity_reference_autocomplete',
      'user_reference_autocomplete' => 'entity_reference_autocomplete',
      'user_reference_url' => 'entity_reference_autocomplete_tags',
    );

    if (in_array($row->getSourceProperty('type'), array('node_reference', 'user_reference'))) {
      $process = $migration->getProcess();
      $process['options/type']['type']['map'] += $widget_map;
      $migration->setProcess($process);
    }
  }

  if ($migration->id() == CUSTOM_MIGRATION_PREFIX . 'd7_field_formatter_settings') {

    // Map the changes in formatter names. This does not update formatter settings.
    // They have to be updated manually after migration.
    $formatter_map = array(
      'node_reference_default' => 'entity_reference_label',
      'node_reference_plain' => 'entity_reference_label',
      'node_reference_nid' => 'entity_reference_entity_id',
      'node_reference_node' => 'entity_reference_entity_view',
      'node_reference_path' => 'entity_reference_label',
      'node_reference_url' => 'entity_reference_label',
      'user_reference_default' => 'entity_reference_label',
      'user_reference_plain' => 'entity_reference_label',
      'user_reference_nid' => 'entity_reference_entity_id',
      'user_reference_node' => 'entity_reference_entity_view',
      'user_reference_path' => 'entity_reference_label',
      'user_reference_url' => 'entity_reference_label',
    );

    if (substr($row->getSourceProperty('type'), 0, 14) == 'node_reference' ||
      substr($row->getSourceProperty('type'), 0, 15) == 'user_reference') {
      $process = $migration->getProcess();
      $process['options/type'][0]['map'] += $formatter_map;
      $migration->setProcess($process);
    }
  }

  // Update all the entity migrations that have nodereference and userreference fields,
  // on this site they're only on nodes, so ids like d7_node will catch them all.
  // This will pick up both the main and the revision migrations.
  $len = strlen('d7_node__') + strlen(CUSTOM_MIGRATION_PREFIX);
  if (substr($migration->id(), 0, $len) == CUSTOM_MIGRATION_PREFIX . 'd7_node__') {

    foreach ($nodereference_fields as $field) {
      if ($value = $row->getSourceProperty($field)) {
        if (!empty($value)) {
          foreach($value as $delta => $item) {
            // Now uses 'target_id' instead of 'nid'
            $value[$delta]['target_id'] = $value[$delta]['nid'];
            $value[$delta]['target_type'] = 'node';
          }
          $row->setSourceProperty($field, $value);
          // Uncomment to debug results.
          //drush_print_r($row);
        }
      }
    }
    foreach ($userreference_fields as $field) {
      if ($value = $row->getSourceProperty($field)) {
        if (!empty($value)) {
          foreach($value as $delta => $item) {
            // Now uses 'target_id' instead of 'uid'
            $value[$delta]['target_id'] = $value[$delta]['uid'];
            $value[$delta]['target_type'] = 'user';
          }
          $row->setSourceProperty($field, $value);
          // Uncomment to debug results.
          //drush_print_r($row);
        }
      }
    }
  }

}
