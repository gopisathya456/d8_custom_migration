<?php
/**
 * @file
 *   Code to convert D7 geolocation to D8 geolocation.
 *
 * Problem being solved: There is no migration path for D7 to D8 geolocation fields.
 */
use Drupal\migrate\Row;
use Drupal\migrate\Plugin\MigrateSourceInterface;
use Drupal\migrate\Plugin\MigrationInterface;

function custom_migration_geolocation(Row $row, MigrateSourceInterface $source, MigrationInterface $migration) {

  // List all the geofield field names.
  $geofields = array(
    'field_geolocation',
  );

  // Map D7 geolocation_latlng to D8 geolocation.
  if ($migration->id() == CUSTOM_MIGRATION_PREFIX . 'd7_field') {
    $process = $migration->getProcess();
    $process['type'][0]['map']['geolocation_latlng'] = 'geolocation';
    $migration->setProcess($process);
  }

  if ($migration->id() == CUSTOM_MIGRATION_PREFIX . 'd7_field_instance_widget_settings') {

    // Map the changes in widget names. This does not update widget settings.
    // They have to be updated manually after migration.
    $widget_map = array(
      'geolocation_googlemap' => 'geolocation_googlegeocoder',
      'geolocation_googlemap_d8' => 'geolocation_googlegeocoder',
      'geolocation_html5_widget' => 'geolocation_html5',
      'geolocation_text' => 'geolocation_latlng',
    );

    if ($row->getSourceProperty('type') == 'geolocation') {
      $process = $migration->getProcess();
      $process['options/type']['type']['map'] += $widget_map;
      $migration->setProcess($process);
    }
  }

  if ($migration->id() == CUSTOM_MIGRATION_PREFIX . 'd7_field_formatter_settings') {

    // Map the changes in formatter names. This does not update formatter settings.
    // They have to be updated manually after migration.
    $formatter_map = array(
      'geolocation_googlemaps_static' => 'geolocation_map',
      'geolocation_googlemaps_dynamic' => 'geolocation_map',
      'geolocation_html5_mapimage' => 'geolocation_map',
      'geolocation_text' => 'geolocation_latlng',
      'geolocation_latlng' => 'geolocation_latlng',
      'geolocation_latitude' => 'geolocation_latlng',
      'geolocation_longitude' => 'geolocation_latlng',
    );

    if (substr($row->getSourceProperty('type'), 0, 11) == 'geolocation') {
      $process = $migration->getProcess();
      $process['options/type'][0]['map'] += $formatter_map;
      $migration->setProcess($process);
    }
  }

  // Update all the entity migrations that have geo fields, on this site it's only nodes.
  // This will pick up both the main and the revision migrations.
  $len = strlen('d7_node__') + strlen(CUSTOM_MIGRATION_PREFIX);
  if (substr($migration->id(), 0, $len) == CUSTOM_MIGRATION_PREFIX . 'd7_node__') {
    foreach ($geofields as $field) {
      if ($value = $row->getSourceProperty($field)) {
        if (!empty($value)) {
          foreach($value as $delta => $item) {

          }
          $row->setSourceProperty($field, $value);
          // Uncomment to debug results.
          //drush_print_r($row);
        }
      }
    }

  }
}