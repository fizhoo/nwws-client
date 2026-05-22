# NWWS Client

NWWS Client receives and stores NWS text bulletins from the NOAA Weather Wire
Service Open Interface (NWWS-OI).

NWWS information: https://www.weather.gov/nwws/

## Description

Current version: `0.3`

NWWS Client is designed for operators who need a simple service that:
- stays connected to NWWS-OI
- writes incoming bulletins to disk
- reconnects automatically when needed
- provides configurable logs for operations and troubleshooting

## Getting Started

Request NWWS-OI access: https://www.weather.gov/nwws/nwws_oi_request

### Dependencies

- `libstrophe-dev` (0.10+)
- `libexpat1-dev`
- `libssl-dev`
- `gcc`
- `make`

### Build

```bash
make            # default debug build (ASan + analyzer)
make debug      # explicit debug build
make release    # optimized build with -DNDEBUG
```

### Run

```bash
./nwws_client <username@...> <nwws_password>
```

Example:

```bash
./nwws_client user@nwws-oi.weather.gov 'your-password'
```

## What The Client Does

- Connects to `nwws-oi.weather.gov:5222` via libstrophe
- Joins `nwws@conference.nwws-oi.weather.gov/<user name>`
- Parses incoming message stanzas and writes valid bulletins to files
- Uses timed health checks and XMPP pings
- Reconnects on disconnect/health-check failure/SIGPIPE unless shutdown is requested

## Bulletin Storage

- Base output directory is currently hardcoded in code.
- Bulletins are written under per-center directories (lowercased `cccc`)
- File naming pattern:
  `cccc-ttaaii_awipsid-id.txt` (lowercased path/filename)

## TODO

- Move hardcoded runtime settings (including output directory and conference
  resource/user name) into a configuration file.

## Monitoring Signal

- If no valid bulletin was written during the alarm interval, the client creates
  `/tmp/nwws_alarm.txt` and sets reconnect requested state.

## Logging

Application logs are written to `stderr` with this format:

```text
[LEVEL] message
```

Supported levels:
- `error`
- `warn` (or `warning`)
- `info`
- `debug`

Set with:

```bash
export NWWS_LOG_LEVEL=debug
```

Default log level:
- debug builds (`make`/`make debug`): `debug`
- release builds (`make release`): `info`

Notes:
- Application logging level only controls messages from `log.c`.
- Libstrophe logging is initialized at `XMPP_LEVEL_INFO` in current code.

## Environment Variables

- `NWWS_LOG_LEVEL`
- `NWWS_PING_INTERVAL_MINUTES`
- `NWWS_ALARM_INTERVAL_SECONDS`

Validation rules currently in code:
- `NWWS_PING_INTERVAL_MINUTES`: integer 1..1440
- `NWWS_ALARM_INTERVAL_SECONDS`: integer 1..86400
- Invalid values generate warnings and fall back to defaults.

## Version History

- `0.3`
- README restructured for customer/operator use
- logging and runtime options documented from current implementation
- `0.2`
- various bug fixes and optimizations
- `0.1`
- initial release

## Authors

Adam Young  
ay@fizhoo.com

Commits:
https://github.com/fizhoo/nwws-client/commits/main

## License

This project is licensed under the MIT License. See `LICENSE`.

## Acknowledgments

- libstrophe examples:
  https://github.com/strophe/libstrophe/tree/master/examples

## Appendix: Developer Notes

### Source Map (Files and Functions)

- `nwws_client.c`: `main`, `sigSetup`, `sigHandler`
- `xmpp_connect.c`: `nwws_connect_new`, `nwws_connect`, `nwws_disconnect`
- `xmpp_connect.c` internals: `stanza_attributes_missing`, `message_handler`,
  `conn_handler`, `nwws_connect_with_retry`, `nwws_run_loop`
- `xmpp_ping.c`: `xmpp_ping_configure_interval`, `xmpp_ping_interval_ms`,
  `xmpp_ping_reset_state`, `xmpp_ping_timed_handler`,
  `xmpp_ping_stanza_handler`
- `alarm_timer.c`: `alarm_interval_ms`, `alarm_timed_handler`
- `file_io.c`: `write_data`
- `text_utils.c`: `chomp`
- `log.c`: `log_init`, `log_set_level`, `log_get_level`, `log_message`,
  `log_level_label`, `level_from_env`
