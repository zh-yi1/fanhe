const fs = require('fs');
const path = require('path');

const cbpPath = path.join(__dirname, '..', 'app.cbp');
let lines = fs.readFileSync(cbpPath, 'utf8').split('\n');
let result = [];
let removed = 0;
let i = 0;

const removeFiles = [
  'func_alarm_clock.c', 'func_alarm_clock_sub_edit.c', 'func_alarm_clock_sub_repeat.c', 'func_alarm_clock_sub_set.c',
  'func_altitude.c', 'func_bird.c',
  'func_blood_oxygen.c', 'func_blood_pressure.c', 'func_bloodsugar.c', 'func_breathe.c', 'func_brightness.c',
  'func_calculator.c', 'func_calculator_alg.c', 'func_calculator_alg.h',
  'func_calendar.c', 'func_camera.c', 'func_compass.c',
  'func_disturd_config.c', 'func_emit_list.c', 'func_findphone.c', 'func_flashlight.c',
  'func_fmrx.c', 'func_fmrx.h',
  'func_game.c', 'func_game_tetris.c', 'func_game_tetris_over.c', 'func_game_tetris_start.c',
  'func_gif.c', 'func_heartrate.c', 'func_light.c',
  'func_menu.c', 'func_menu.h',
  'func_menu_sub_cum_sudoku.c', 'func_menu_sub_disk.c', 'func_menu_sub_football.c',
  'func_menu_sub_grid.c', 'func_menu_sub_hexagon.c', 'func_menu_sub_honeycomb.c',
  'func_menu_sub_list.c', 'func_menu_sub_ring.c', 'func_menu_sub_skyrer.c',
  'func_menu_sub_sudoku.c', 'func_menu_sub_waterfall.c',
  'func_menustyle.c', 'func_message.c', 'func_music.h',
  'func_passwod_sub_disp.c', 'func_password_sub_select.c', 'func_photo_view.c',
  'func_recorder.c', 'func_recorder.h', 'func_scan.c',
  'func_setting_sub_about.c', 'func_setting_sub_disturd.c', 'func_setting_sub_dousing.c',
  'func_setting_sub_language.c', 'func_setting_sub_menu_navigation.c', 'func_setting_sub_off.c',
  'func_setting_sub_password.c', 'func_setting_sub_restart.c', 'func_setting_sub_rstfy.c',
  'func_setting_sub_sav.c', 'func_setting_sub_time.c', 'func_setting_sub_wrist.c',
  'func_sleep.c', 'func_smartstack.c',
  'func_sport.c', 'func_sport_config.c', 'func_sport_sub_run.c', 'func_sport_switching.c',
  'func_stopwatch.c', 'func_stopwatch_sub_record.c', 'func_style.c',
  'func_take_photo.c', 'func_timer.c',
  'func_usbdev.c', 'func_usbdev.h',
  'func_video_showlist.c', 'func_voice.c', 'func_volume.c', 'func_weather.c',
  'func_address_book.c', 'func_time_sub_custom.c',
];

while (i < lines.length) {
  const line = lines[i];
  const stripped = line.trim();

  if (stripped.startsWith('<Unit')) {
    let matched = false;
    for (const f of removeFiles) {
      if (line.includes('functions/' + f)) {
        if (stripped.endsWith('/>')) {
          removed++;
          matched = true;
        } else {
          removed++;
          i++;
          while (i < lines.length && !lines[i].trim().startsWith('</Unit>')) { removed++; i++; }
          if (i < lines.length) { removed++; i++; }
          matched = true;
        }
        break;
      }
    }
    if (matched) continue;
  }

  result.push(line);
  i++;
}

fs.writeFileSync(cbpPath, result.join('\n'));
console.log('Done! Removed ' + removed + ' lines. Remaining: ' + result.length);
