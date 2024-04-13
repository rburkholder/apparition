# Apparition - Home/Office Automation/Monitoring

_an immaterial appearance that seems real, and is generally sudden or startling in its manifestation -- something akin to what a monitoring and automation solution may appear to be_

The philosophy with Apparition is to compose an automation/monitoring/alert tool from a series of mature packages, all integrated under one roof.  The tool should work in the background and only manifest itself when needed.

Functionality:
* C++ core for coordinating the subsystems
* Lua scripting to handle parsing, variable aggregation, and event handling
  * Scripts can be live edited without restarting the application
* Current implementation sensors decoded & controlled via Lua:
  * zwave controller via Zooz & Z-Wave JS UI (MQTT)
    * Honeywell Thermostats
    * GIG Thermostats
    * Zooz Scene controllers controls lights
    * New-One metered smartplug
    * smoke alarm
  * zigbee controller via Sonoff & Zigbee2MQTT (MQTT)
    * Philips Hue lights
    * PIR sensors
  * RTLSDR 433Mhz & 915Mhz radios (MQTT)
    * EcoWitt WS90 Weather Station
    * DSC PIR, Door, Smoke alarm
    * Thermopro temperature/humidity
    * Neptune water meter
  * BeagleBoard BME 680 (MQTT)
    * temperature, humidity, pressure
* Time series data sent to Prometheus for efficient recording
  * over 120 time series currently collected so storage efficiency is important
* Grafana tied to Prometheus for dashboards and charting
* A simple web dashboard with current values

Operation:
* update the var/apparition.cfg with your specific requirements
* start the application using ./var as your working directory

On the immediate todo list:
* dashboard updates:
  * organize devices & sensors logically - defined by Lua config statements
  * control devices using widgets - triggering Lua control scripts
* provide persistence via an attached database, probably sqlite
* embed charts of prometheus data in dashboard
* improved recovery from failed scripts
* add time-of-day and recurring events

Order of install:
* need to build and install https://github.com/rburkholder/repertory for MQTT
* install packages:
  * [C++ library: mqtt-paho](docs/mqtt-paho.md) - mqtt client
* build libs-build libraries - use [libs-build](https://github.com/rburkholder/libs-build)
  * [C++ library: boost](docs/boost.md) - asio, datetime, fusion, spirit
  * [C++ library: wt](docs/wt.md) - web page rendering, database access, depends upon boost
* build 3rdparty tools:
  * [C++ library: fmt](docs/fmt.md) - fast format library
  * [C++ library: luajit](docs/lua.md) - compiled lua scripts
  * [C++ library: lua-cjson](docs/lua.md) - json encoder/decoder
  * [C++ library: prometheus-cpp](docs/prometheus-cpp.md) - exporter development kit
  * [C++ library: yaml](docs/yaml-cpp.md) - yaml config processing
* install applications (in container or locally):
  * [prometheus](docs/prometheus.md) - timeseries collection
  * [grafana](docs/grafana.md) - dashboards
  * [rabbitmq](docs/rabbitmq) - mqtt broker
* build apparition:
  * [notes](docs/apparition.md)

Proposed tooling:
* C++20 as primary backend
  * [boost](https://www.boost.org/) - use my [libs-build](https://github.com/rburkholder/libs-build) library to bulid and install
    * [asio](https://www.boost.org/doc/libs/1_82_0/doc/html/boost_asio.html) - signals, network
    * [beast](https://www.boost.org/doc/libs/1_82_0/libs/beast/doc/html/index.html) - REST, WebSocket
  * [jwt-cpp](https://thalhammer.github.io/jwt-cpp/), [cpp-jwt](https://github.com/arun11299/cpp-jwt) - consideration for javascript websocket tokens
  * [paho mqtt](https://github.com/eclipse/paho.mqtt.c) - simplified installation via Debian package repository
  * [wt](https://www.webtoolkit.eu/wt) - server side REST/UI
  * [yaml](https://github.com/jbeder/yaml-cpp) - YAML in/out for config files
    * [installation notes](docs/yaml-cpp.md)
    * [tutorial](https://github.com/jbeder/yaml-cpp/wiki/Tutorial)
    * [blog entry](https://www.fatalerrors.org/a/c-read-and-write-yaml-configuration-file.html)
  * Candidate embedded scripting language:
    * [Duktape](https://duktape.org/guide.html) - embeddable ECMAScript® engine with a focus on portability and compact footprint
    * [LuaJIT](https://luajit.org/) - Just-In-Time Compiler for Lua - currently in trials
* [Bootstrap](https://getbootstrap.com/) - web page dynamics
* [Node-RED](https://nodered.org/) - visual event editor, used in addition to the embedded scripting language
* [Prometheus](https://prometheus.io/docs/introduction/overview/) - time series open-source systems monitoring and alerting toolkit
* [Grafana](https://grafana.com/) - data visualization aka visibility stack
  * [github](https://github.com/grafana/grafana)
* [RabbitMQ / MQTT](https://www.rabbitmq.com/mqtt.html) - event backbone
* [Rhasspy](https://github.com/rhasspy) - Offline voice assistant [docs](https://rhasspy.readthedocs.io/en/latest/)
  * [ESP32-Rhasspy-Satellite](https://github.com/Romkabouter/ESP32-Rhasspy-Satellite) - esp32 standalone MQTT audio streamer. Is is desinged to work as a satellite for Rhasspy

Thoughts influencing the design & implementation of this solution:

* config is supplied through a series of yaml files, which are primarily used to load scripts and attach the scripts to mqtt events
  * system configuration is thus easily version controlled, replaceable, and reproducible
* Prometheus supplies time series collection & management for historical review.  It offers compaction, particularly needed for sensors, such as the Ecowitt WS90 which issues multiple variables every 8 seconds or so.
* scripting language provides a rich set of interactive text based scripting capabilities for handling events, triggers, and logic
* MQTT broker to act as a backbone for messaging between sub-systems
* some draw backs of other solutions:
  * Home Assistant has a rather rigidily enforced installation if you want the base plus the store.  Plus there are comments that breakage occurs frequently during release cylces.
  * Domoticz is totally self contained and fast.  But it is missing certain signals such as the operational state of a thermstat.  The charts, based upon my limited experience, are not very informative.

