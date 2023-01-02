/*
 * blupig
 * Copyright (C) 2016-2017 Yunzhu Li
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

var express = require('express');
var cors = require('cors');
var exec = require('child_process').exec;

console.log('Start listening...');
start();

function start() {
  const corsOptions = {
    origin: 'https://www.lrnctu.tw',
  }

  var app = express();
  app.use(cors());

  app.get('/status', function (req, res) {
    res.send('ok');
  });

  // Compute move
  app.get('/move', function (req, res) {
    // Get query parameters
    var state = req.query.s;
    var player = req.query.p;
    var max_depth = req.query.d;
    var max_time = req.query.l;
    var threads = req.query.t;
    var algorithm = req.query.a;

    // Build command
    var cmd = 'gomoku';
    if (typeof state !== 'undefined' && state.length > 0) cmd += ' -s ' + state;
    if (typeof player !== 'undefined' && player.length > 0) cmd += ' -p ' + player;
    if (typeof max_depth !== 'undefined' && max_depth.length > 0) cmd += ' -d ' + max_depth;
    if (typeof max_time !== 'undefined' && max_time.length > 0) cmd += ' -l ' + max_time;
    if (typeof threads !== 'undefined' && threads.length > 0) cmd += ' -t ' + threads;
    if (typeof threads !== 'undefined' && threads.length > 0) cmd += ' -a ' + algorithm;

    console.log(cmd);

    // Execute command
    exec(cmd, function(error, stdout, stderr) {
      console.log(stdout);

      // Write response
      res.write(stdout);
      res.end();
    });
  });
  app.listen(8001);
}
