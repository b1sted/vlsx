# vlsx C99 Engineering Style Guide & Architecture Guidelines

This document defines the coding, architecture, and safety rules for the
`vlsx` codebase: a terminal toolkit for network engineers built in C99 on top
of termbox2. It is written as an engineering regulation, not a suggestion.
`clang-format` and the build enforce the mechanical parts; reviewers enforce
layering, ownership, and behavior.

The layer boundaries below are the target architecture. When the current tree
does not match them, follow the target rules here and treat the old layout as
transitional.

The project baseline is C99. The guide deliberately borrows the discipline of
production C99/C11 codebases (Linux, OpenBSD, SEI CERT C, SQLite, Git, Redis):
strict namespacing, explicit API contracts, single-exit cleanup, and a
warning-free build. The rules are compatible with both C99 and C11, but the
repository currently uses `-std=gnu99`. Moving to `-std=c11` is a dedicated
build change; C11 features are not adopted piecemeal.

## 1. Language Standard

- The project builds with **GNU C99** (`-std=gnu99`) and the POSIX/XSI feature
  macros `-D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE`. Write C99-compatible code;
  the GNU/POSIX environment exists because termbox2 and the platform headers
  need it, not as permission to use non-portable extensions.
- Do **not** use C11 or later features unless the project explicitly moves its
  baseline in a dedicated build change. In particular, the following are
  banned:
  - `_Generic`
  - anonymous structs and anonymous unions
  - `<threads.h>`
  - `<stdatomic.h>`
  - `_Static_assert`
  - variable length arrays (VLAs)
- Any array whose size depends on runtime data must be allocated on the heap or
  bounded by a compile-time constant. Reuse the limits in `include/input.h`
  (`TINY_BUFFER`, `MINI_BUFFER`, `SMALL_BUFFER`, `MEDIUM_BUFFER`, …) or the
  future `common/limits.h`, and always pass the destination capacity explicitly.
- Do not add GNU extensions of your own. GNU statement expressions, nested
  functions, and `typeof` are not permitted.
- Every source and header file starts with the SPDX identifier:
  `/* SPDX-License-Identifier: GPL-3.0-or-later */`.

## 2. Repository Architecture

The code is split into layers. A layer may depend only on the layers to its
left; the TUI may read everything below it but must not mutate it.

| Layer       | Responsibility                                              | May depend on                          | Must not depend on                                 |
|-------------|-------------------------------------------------------------|----------------------------------------|----------------------------------------------------|
| `common/`   | DFA, numeric/string helpers, input model, limits, UI events  | C standard library                     | `core/`, `screens/`, `app/`, `tui/`, termbox2      |
| `core/`     | Network calculations: IPv4, VLSM, MAC, SLA, DHCP            | `common/`                              | termbox2, `app`, `screens`, `tui`, terminal output |
| `screens/`  | Screen state, controllers, semantic event handling           | `core/`, `common/`                     | termbox2, `tb_*`, drawing coordinates              |
| `app/`      | Global state, focus, routing, menus, top-level events        | `screens/`, `core/`, `common/`         | `tb_*`, rendering                                  |
| `tui/`      | termbox2 backend, theme, layout, primitives, table, views    | `app/`, `screens/`, `core/`, `common/` | changing application or screen state               |
| `main.c`    | Initialization, main loop, wiring of `app/` and `tui/`       | all layers                             | business logic                                     |

Target tree:

```
include/                 public headers, mirrored by layer
├── app/
├── common/
├── core/
├── screens/
└── tui/
src/
├── main.c
├── common/
│   ├── dfa.c
│   ├── utils.c
│   ├── limits.h                    private if not shared
│   ├── input_field.c
│   └── ui_event.h
├── core/
│   ├── net/ipv4/
│   │   ├── ipv4_types.h
│   │   ├── ipv4_validate.c
│   │   ├── ipv4_parse.c
│   │   ├── ipv4_calc.c
│   │   └── ipv4_class.c
│   └── net/vlsm/
│       ├── hosts_list.c
│       ├── vlsm_alloc.c
│       └── vlsm_types.h
├── screens/
│   ├── ip/
│   │   ├── ip_screen.c
│   │   ├── ipv4_screen.c
│   │   └── vlsm_screen.c
│   ├── l2/
│   ├── sla/
│   └── dhcp/
├── app/
│   ├── app.c
│   ├── router.c
│   ├── events.c
│   └── menus.c
└── tui/
    ├── backend.c
    ├── theme.h
    ├── layout.c
    ├── primitives.c
    ├── table.c
    ├── chrome.c
    ├── render.c
    └── views/
        └── ip/
            ├── ip_tabbar.c
            ├── ipv4_view.c
            └── vlsm_view.c
```

Public headers live under `include/` with the same relative path as their
implementation. Private headers may live next to their `.c` file under `src/`.
`-Iinclude` keeps includes such as
`#include "core/net/ipv4/ipv4_calc.h"` working. Do not move public headers next
to their sources unless the project explicitly adopts that layout.

Layering rules:

- `common/` and `core/` never include `app.h`, `ui.h`, `render.h`,
  `termbox2.h`, anything from `screens/`, or anything from `tui/`.
- `core/` never reads or writes the global `app` state and never formats
  terminal output.
- `screens/` never call `tb_*` and never include `termbox2.h`.
- `app/` never calls `tb_*`.
- `tui/` may read application state and calculation results, but rendering is
  read-only: it must not assign to `app`, screen models, or calculation results.
- `TB_IMPL` is defined exactly once, in `tui/backend.c`. During migration, the
  single temporary exception is `main.c`.
- `main.c` performs initialization and the event loop; it contains no network
  or screen-specific business logic.

The repository is mid-migration: some modules still live directly under `src/`
(`vlsx.c`, `app.c`, `events.c`, `render.c`, `ui.c`, `input.c`, `dfa.c`,
`utils.c`). Do not add new modules there; put new code in the target layer and
move an existing module when a change already touches its internals.

### 2.1. Encapsulation and opaque pointers

Do not expose structure fields in a public header unless callers genuinely need
them. For modules with hidden state or invariants, declare an opaque type in
the public header and keep the definition in the implementation:

```c
/* include/core/net/vlsm/vlsm_plan.h */
typedef struct vlsm_plan vlsm_plan_t;

vlsm_plan_t *vlsm_plan_create(void);
void vlsm_plan_destroy(vlsm_plan_t *plan);
bool vlsm_plan_build(vlsm_plan_t *plan, const char *cidr_text,
                     const char *hosts_text, calc_status_t *status);
```

```c
/* src/core/net/vlsm/vlsm_plan.c */
struct vlsm_plan {
    vlsm_result_t result;
    bool has_result;
};
```

Small value types that are intentionally passed by value or embedded by other
modules (`input_field_t`, `v4_addr_t`, `layout_t`) may remain transparent.
Opaque pointers are preferred when a structure is large, owns resources, or has
invariants that callers must not break.

## 3. Naming and Namespacing

- All identifiers use strict `snake_case`. `camelCase` and `PascalCase` are
  forbidden, except for third-party termbox2 names.
- Every non-static function, type, and global carries a module prefix. The
  prefix identifies the owning module, not just the file name.
- Public functions follow `module_entity_action()`:
  `ipv4_calculate()`, `vlsm_plan_build()`, `table_max_row_offset()`,
  `input_insert()`.
- Static functions may omit the module prefix when the file is small and the
  name is unambiguous (`parse_hosts`, `sort_descending`). Do not start
  identifiers with an underscore at file scope; such names are reserved for the
  implementation.

  | Prefix              | Module                                                     |
  |---------------------|------------------------------------------------------------|
  | `dfa_`              | DFA engine                                                 |
  | `util_`             | Generic numeric and string helpers                         |
  | `input_`            | Input field model                                          |
  | `ipv4_`             | IPv4 validation, parsing, calculation, class               |
  | `ipv6_`             | IPv6 formatting and calculation                            |
  | `vlsm_`             | VLSM allocation                                            |
  | `hosts_`            | Host-list parsing and sorting                              |
  | `mac_`              | Layer 2 and multicast MAC tools                            |
  | `sla_`              | SLA, goodput, and cost metrics                             |
  | `dhcp_`             | DHCP timers                                                |
  | `screen_`           | Shared screen dispatch and routing helpers                 |
  | `<feature>_screen_` | A specific screen controller (for example `ipv4_screen_`)  |
  | `app_`              | Global application state and routing                       |
  | `ui_`               | TUI primitives                                             |
  | `render_`           | Frame composition and chrome                               |
  | `table_`            | Table model, metrics, and drawing                          |
  | `layout_`           | Terminal layout calculation                                |
  | `<feature>_view_`   | A specific TUI view (for example `ipv4_view_`)             |

- During migration, existing symbols keep their current names (`v4_*`,
  `calc_v4_*`, `run_dfa`, `round_up_pow2`, `string_combine`). Rename them when
  their module moves into the target layer, not in an unrelated change. New
  code in the target layer uses the prefixes above.
- `tb_` and `TB_*` are reserved for termbox2. Do not create project symbols
  with that prefix.
- Type names use `snake_case` with a `_t` suffix (`input_field_t`,
  `app_state_t`, `vlsm_entry_t`). Use `typedef struct vlsm_entry vlsm_entry_t;`
  for opaque types.
- Enum type names end in `_t`. Enum members use `SCREAMING_SNAKE_CASE` with a
  module prefix: `V4_S_START`, `HOSTS_C_DIGIT`, `VLSM_COL_NETWORK`,
  `APP_SCR_IP`, `CALC_ERR_RANGE`.
- Macros and compile-time constants use `SCREAMING_SNAKE_CASE`. Project-wide
  macros use the `VLSX_` prefix (`VLSX_ARRAY_SIZE`, `VLSX_MAX_PATH_LEN`);
  module-local macros use the module prefix (`MAX_SUBNETS`, `V4_INPUT_MAX`,
  `COLOR_COBALT`).
- `main` is the only function exempt from the prefix rule.

## 4. Formatting

Formatting is enforced by `clang-format` using the repository `.clang-format`.
The target configuration uses an LLVM base with 4-space indentation, a
100-column limit, function braces on their own line, and mandatory braces.

- Indent with **4 spaces**. Never use tab characters in C files.
- Maximum line length is **100 columns**.
- Braces follow K&R/Linux style:
  - Function definitions place the opening brace on its **own line**.
  - Control statements (`if`, `for`, `while`, `switch`, `do`) place the
    opening brace on the same line.
  - `struct`, `enum`, and `union` bodies place the opening brace on the same
    line.
  - `else` stays on the same line as the closing brace.
- **Braces are mandatory for every body**, including single-statement
  `if`/`else`, loops, and `do`/`while`. The formatter is configured with
  `InsertBraces: true` (clang-format 13+); do not rely on a reviewer to catch
  missing braces.
- Pointer placement attaches the asterisk to the identifier:
  `char *buf`, not `char* buf`; `const char *name`, not `const char* name`;
  `const char *const *list`.
- Do not align consecutive assignments or declarations.
- One statement per line. Do not use the comma operator to pack statements.
- Comments are part of the source of truth. Automated formatting may re-align
  comments but must never change their text.

Format touched files before committing:

```bash
clang-format -i path/to/file.c path/to/file.h
clang-format --dry-run --Werror path/to/file.c path/to/file.h
```

The repository is migrating from the old 80-column/same-line-brace layout to
the configuration above. A full-tree reformat belongs in a dedicated `style:`
commit; do not mix it with a behavior change. Until that commit lands, expect
the formatter to adjust a whole file when you touch it. If a rule in this
section conflicts with `.clang-format`, the formatter wins and this document
must be updated.

## 5. Types and Data Discipline

- Include `<stdint.h>` and use fixed-width types for all data that crosses a
  module, file, syscall, or hardware boundary: `uint8_t`, `uint16_t`,
  `uint32_t`, `uint64_t`, `int8_t`, `int16_t`, `int32_t`, `int64_t`.
- Use `uint32_t` for IPv4 address values and `uint8_t` for octets and prefix
  lengths.
- Use `size_t` for object sizes, buffer capacities, indices, and counts.
  Use `ssize_t` when a POSIX API returns a signed byte count or a signed size.
- Use `uintptr_t` and `intptr_t` for pointer-valued integer arithmetic and
  round-trips. Do not cast pointers to `long` or `unsigned long`.
- Use `bool` from `<stdbool.h>` for predicates, flags, and yes/no results.
  Use a status enum for operations that can fail for different reasons.
- Plain `int` is allowed for:
  - function status codes and small enum-like values;
  - `argc` and `argv` processing;
  - trivial loop counters over compile-time-bounded ranges;
  - terminal coordinates, widths, heights, and other TUI geometry.
- Raw `long`, `unsigned long`, `short`, and `unsigned short` must not be used
  for stored data. When a libc or POSIX API requires them (for example
  `strtol`, `sysconf`, or `ftell`), convert at the call site and immediately
  narrow to a fixed-width type with an explicit range check.
- Do not use bare `unsigned` or `signed`; write the exact type.
- Always cast explicitly when narrowing. Never rely on implicit truncation.
- Use `PRIu32`, `PRId32`, and the other `<inttypes.h>` macros when formatting
  fixed-width types.
- Floating-point values are limited to metrics and display formatting (for
  example, utilization percentage); do not use them for addresses, masks,
  identifiers, counters, or flags.
- `uintattr_t` and other termbox2 types belong to `tui/` only. Calculation and
  screen layers use plain integer types.

## 6. Const Correctness

- Mark pointer parameters `const` whenever the callee does not modify the
  pointee: `const char *path`, `const uint8_t *data`.
- Mark pointer-to-pointer arrays `const` at the appropriate level, for example
  `const char *const items[]`.
- Functions that return string literals return `const char *`:
  `const char *ipv4_class_name(ipv4_class_t cls)`.
- Apply `const` only when it is safe. Do not add `const` when the address is
  passed to an API that expects a mutable pointer and would emit a
  `discarded-qualifiers` warning.
- Prefer `const` on locals that are assigned once and never modified.
- Unchanged scalar parameters may be marked `const` (the existing screens do
  this for coordinates). Be consistent within a file.
- Never cast away `const` to silence a warning. Fix the interface instead.

## 7. Initialization, Ownership, and Memory Safety

- Initialize every variable at declaration. Pointers default to `NULL`,
  scalars to `0`, and structures to `{0}`.
- Prefer `{0}` followed by explicit assignments for partially populated
  structures. When using designated initializers, initialize every field that
  matters.
- Check every allocation, every syscall result, and every pointer returned by a
  library function before use. This includes `strdup`, `malloc`, `open`,
  `read`, and termbox2 calls that can fail.
- Document ownership transfer. State clearly whether a function takes ownership
  of a pointer, borrows it, or returns newly allocated memory that the caller
  must free.
- `free()` accepts `NULL`; set pointers to `NULL` after freeing when the
  variable remains in scope. Do not free the same pointer twice.
- If a project-wide helper is introduced for the common pattern, keep it small
  and safe:

  ```c
  #define VLSX_SAFE_FREE(ptr) \
      do {                      \
          free(ptr);            \
          (ptr) = NULL;         \
      } while (0)
  ```

  The argument must be a mutable lvalue. Never use this macro on a `const`
  pointer or on an expression with side effects.
- Never index an array without proving the index is within bounds.
- Use `snprintf` rather than `sprintf`, and always pass the destination
  capacity. Do not use `strcpy`; prefer `snprintf`, `memcpy` with a proven
  length, or the project string helpers.
- Do not ignore return values from `read`, `write`, `open`, `close`, I/O, or
  allocation functions in production paths. A single `read()` or `write()` may
  transfer fewer bytes than requested; loop or explicitly handle the short
  transfer.
- Capture `errno` immediately after a failing libc/POSIX call if the error will
  be reported later. Do not call other functions before saving it.

### 7.1. Single exit point and `goto cleanup`

Functions that acquire more than one resource (file descriptors, heap
allocations, locks, platform handles) use a single exit point and unwind in
LIFO order. Status is carried in one variable; data is returned through an
out-parameter.

```c
typedef enum {
    VLSX_OK = 0,
    VLSX_ERR_INVAL,
    VLSX_ERR_NOMEM,
    VLSX_ERR_IO,
} vlsx_status_t;

vlsx_status_t example(const char *path, result_t *out) {
    vlsx_status_t status = VLSX_ERR_NOMEM;
    int fd = -1;
    char *buf = NULL;

    if (path == NULL || out == NULL) {
        return VLSX_ERR_INVAL;
    }

    buf = malloc(BUFFER_SIZE);
    if (buf == NULL) {
        goto cleanup;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        status = VLSX_ERR_IO;
        goto cleanup;
    }

    if (read(fd, buf, BUFFER_SIZE) < 0) {
        status = VLSX_ERR_IO;
        goto cleanup;
    }

    out->data = buf;
    buf = NULL; /* ownership transferred to out */
    status = VLSX_OK;

cleanup:
    if (fd >= 0) {
        if (close(fd) < 0 && status == VLSX_OK) {
            status = VLSX_ERR_IO;
        }
    }
    free(buf);
    return status;
}
```

- The `cleanup` label is the last statement before the return.
- Release resources in the reverse order of acquisition.
- Guard cleanup with checks so it is safe on partially initialized paths.
- A `fail:` label may be used for validation errors when the function already
  has a cleanup path; make sure every resource is released on all paths.
- Do not use `goto` for ordinary control flow; it is reserved for cleanup.

## 8. Header Organization

Within every `.c` file, include headers in this order, separated by blank
lines:

1. The module's own header (for example `#include "core/net/ipv4/ipv4_calc.h"`).
2. C standard headers (`<stdbool.h>`, `<stdint.h>`, `<stdio.h>`, ...).
3. POSIX/platform headers (`<fcntl.h>`, `<unistd.h>`, `<sys/...>`, ...).
4. Third-party headers (`"termbox2.h"`; only inside `src/tui/` after the
   migration).
5. Project headers from other modules, in dependency order.

Including the module's own header first makes that header self-contained and
catches missing includes immediately.

```c
/* src/tui/views/ip/ipv4_view.c */
#include "tui/views/ip/ipv4_view.h"

#include <stdbool.h>
#include <stdint.h>

#include "app/app.h"
#include "core/net/ipv4/ipv4_calc.h"
#include "tui/layout.h"
```

Headers use include guards rather than `#pragma once`. The guard name is the
full path in `SCREAMING_SNAKE_CASE` with a `_H` suffix:

```c
#ifndef VLSX_CORE_NET_IPV4_CALC_H
#define VLSX_CORE_NET_IPV4_CALC_H

/* ... */

#endif /* VLSX_CORE_NET_IPV4_CALC_H */
```

Existing short guards such as `DFA_H` may remain during migration; rename them
when the file is moved into the target layer. Headers must be self-contained:
each header includes everything it needs. Do not include a header that is not
used by the translation unit. Forward-declare types in headers when only a
pointer is needed. Do not create header cycles; extract shared types into a
third header instead.

## 9. Functions and Control Flow

- Keep functions focused and short enough to review. Extract helpers when a
  function grows beyond a single responsibility. A screen controller should
  not also be the calculator, the table renderer, and the scroll handler.
- Validate all parameters at the top of public functions and return early on
  invalid input.
- Prefer early returns for error checks. Use `goto cleanup` only for resource
  unwinding as described above.
- Every `switch` handles all enum values, either with explicit cases or a
  `default` that fails safely. Do not add an empty `default` just to silence
  `-Wswitch`.
- Do not use assignments inside conditions. Assign first, then test.
- Avoid macros that hide control flow. Prefer `static inline` functions for
  small helpers when a macro is not required.
- Keep the main loop in `main.c`; screen and calculation modules must not poll
  termbox2 themselves.
- Mark every function and global that is not part of the module API `static`.
  A symbol without a prototype or without a reason to be exported is a
  maintenance liability.

### 9.1. `assert()` rules

- Use `assert()` only to detect programmer bugs: broken invariants, impossible
  states, and violated internal contracts.
- Never use `assert()` for runtime errors: invalid user input, missing files,
  allocation failure, permission errors, or network failures. Those must be
  handled through return values and status codes.
- Do not put side effects inside `assert()` arguments; `NDEBUG` removes the
  whole expression.
- Public functions still validate `NULL` pointers and invalid arguments and
  return an error; `assert()` does not replace validation.

## 10. Error Handling and API Contracts

- A function returns one of:
  - a **status** value;
  - a **pointer** to a new object, or `NULL` on failure;
  - a `bool` success flag plus a status out-parameter when both the yes/no
    result and the failure reason are useful.
- If a function computes a value, write the result through an out-parameter
  instead of returning a partially valid structure.
- If it is a factory, return a pointer to the new object or `NULL` on failure.
- Infrastructure code uses the small project status type `vlsx_status_t`
  (defined in §7.1) with `VLSX_OK`, `VLSX_ERR_INVAL`, `VLSX_ERR_NOMEM`, and
  `VLSX_ERR_IO`. Do not overload `bool` for operations that have several
  failure modes.
- Calculation functions use the domain status type `calc_status_t`:

  ```c
  typedef enum {
      CALC_OK = 0,
      CALC_ERR_SYNTAX,
      CALC_ERR_RANGE,
      CALC_ERR_CAPACITY,
      CALC_ERR_ALLOCATION,
  } calc_error_t;

  typedef struct {
      bool ok;
      calc_error_t code;
      char message[MEDIUM_BUFFER]; /* transitional; prefer code + parameters */
  } calc_status_t;
  ```

- Prefer an out-parameter over returning a partially valid structure:
  `bool ipv4_calculate(const char *text, ipv4_result_t *out, calc_status_t *status);`
- Do not use a global flag such as `app.is_input_correct` as an error channel.
  The caller owns the status and decides how to present it.
- Use `bool` only for predicates and yes/no decisions. Operations that can fail
  for different reasons use a status enum.
- Check `NULL` at the top of every public function that accepts pointers and
  return an error. Do not dereference a pointer before validating it.
- Error messages are plain text. Calculation and screen layers do not embed
  colors, coordinates, terminal escapes, or view-specific padding in them.
- The view owns the presentation of an error. During migration, a ready-made
  message is acceptable; the target is an error code plus parameters.
- `errno` is not a general-purpose error channel for project APIs. Capture it
  only when wrapping a libc/POSIX call, and translate it into the module's
  status type.

## 11. Layer Contracts

These contracts are the reason the layers exist. They are not optional.

### 11.1. Calculation results

- `core/` functions must not read or write `app.is_input_correct`, cursor
  values, focus, or any other UI state.
- The screen controller stores `calc_status_t` in its own model and decides
  what to do next.
- Calculation output structures contain addresses, prefixes, counts, and
  canonical address strings. They do not contain coordinates, colors, table
  column widths, or view-specific padding.

### 11.2. Semantic events

Screen headers and `app/events.h` must not expose `struct tb_event`:

```c
typedef enum {
    UI_EVENT_KEY_UP,
    UI_EVENT_KEY_DOWN,
    UI_EVENT_KEY_LEFT,
    UI_EVENT_KEY_RIGHT,
    UI_EVENT_KEY_ENTER,
    UI_EVENT_KEY_ESC,
    UI_EVENT_KEY_BACKSPACE,
    UI_EVENT_KEY_CHAR,
    UI_EVENT_KEY_CTRL,
} ui_event_kind_t;

typedef struct {
    ui_event_kind_t kind;
    uint32_t ch;   /* for UI_EVENT_KEY_CHAR */
    uint32_t ctrl; /* for UI_EVENT_KEY_CTRL, when needed */
} ui_event_t;
```

`tui/backend.c` is the only place that converts `struct tb_event` into
`ui_event_t`. `app/events.c` and `screens/` work only with `ui_event_t`, so
termbox2 can be replaced without touching navigation logic.

### 11.3. Screens and views

A screen owns its input fields and result state and exposes a controller API:

```c
typedef struct {
    input_field_t input;
    bool has_result;
    bool is_valid;
    calc_status_t status;
    ipv4_result_t result;
} ipv4_screen_t;

void ipv4_screen_init(ipv4_screen_t *screen);
void ipv4_screen_confirm(ipv4_screen_t *screen);
bool ipv4_screen_handle(ipv4_screen_t *screen, const ui_event_t *event);
const input_field_t *ipv4_screen_active_input(const ipv4_screen_t *screen);
```

A view only reads the model and draws:

```c
void ipv4_view_render(const ipv4_screen_t *screen, const layout_t *layout);
```

- Views must not call `*_screen_confirm()`, write to the screen model, change
  `app`, or own calculation results.
- Keyboard scrolling and table offsets are view state. They must not live in
  the calculation module.
- For large features, use three files: `core/<feature>_calc.c`,
  `screens/<feature>_screen.c`, `tui/views/<feature>_view.c`. Small features
  may combine the screen and view, but `tb_*` still belongs only in `tui/`.

### 11.4. Layout

Terminal geometry belongs to the TUI. `win_sizes_t` is a temporary bridge;
new code passes an explicit layout instead of reading a global `sizes` from
calculation or screen code:

```c
typedef struct {
    int win_x, win_y, win_w, win_h;
    int content_x, content_y, content_w, content_h;
} layout_t;

void layout_calculate(int term_w, int term_h, layout_t *out);
```

Calculate the layout once at the start of a frame and pass it down. Screens
must not compute window geometry from termbox2 globals.

### 11.5. State ownership

- Each screen has one owner and one reset path. Before a new calculation,
  reset `has_result`, `is_valid`, error text, table rows, and scroll offsets in
  one place.
- Never leave a validity flag set after a failed calculation. If the input
  becomes invalid, the screen must not display the previous table or result.
- Rendering must not be required to initialize state. If a value is needed by
  navigation, initialize it before the frame, not during drawing.

### 11.6. Boundary checks

The boundary rules are intended to be mechanical. When `check-layers` exists
in the Makefile, it must pass. Until then:

- `grep -rn 'termbox2\.h' src` must not report files outside `src/tui/`
  (with the temporary `main.c` exception while `tui/backend.c` does not yet
  exist).
- `grep -rn '\bapp\.' src/core src/common` must find nothing.
- `grep -rn '#include "\(app\|ui\|render\|input\)\.h"' src/core src/common`
  must find nothing.
- Any new dependency that violates a row of the layer table is a design
  error, not a formatting issue.

## 12. Diagnostics and Output

- User-facing output goes through the TUI views and the `ui_` primitives so
  labels, colors, and layout stay consistent.
- Fatal initialization and event-loop failures use `stderr` with the `[ERROR]`
  prefix, as `main.c` does for `tb_init()` and `tb_poll_event()`.
- Calculation and screen modules do not print with `printf`, `fprintf`, or
  termbox2. They return data or a `calc_status_t`.
- Error messages that are temporarily stored in calculation structures must be
  plain text without colors, coordinates, or terminal escape sequences.
- Output must remain stable across formatting-only changes. Formatting and
  brace changes must never change CLI or TUI output.

## 13. Toolchain and Quality Gates

The project must build without warnings. The current `Makefile` enables
`-Wall -Wextra`; the production target is the following set:

```makefile
CFLAGS += -std=gnu99 \
          -Wall -Wextra -Werror \
          -pedantic \
          -Wshadow \
          -Wconversion \
          -Wstrict-prototypes \
          -Wmissing-prototypes \
          -Wformat=2 \
          -Wundef \
          -fno-common
```

- Add new warning flags incrementally and fix the warnings they reveal. Do not
  suppress a diagnostic globally or disable `-Werror` to get a build through.
- `-Wstrict-prototypes` requires `void foo(void)` rather than `void foo()`.
- `-Wmissing-prototypes` requires a declaration for every non-static function.
- `-Wconversion` is strict; explicit casts with checked ranges are the correct
  response, not a blanket disable.
- `-pedantic` may need third-party headers isolated behind `-isystem` before it
  can be enabled cleanly.
- The final build must be warning-free with the full set. A warning is a bug.

Quality gates:

- `clang-format` on touched files; the repository config is authoritative.
- AddressSanitizer and UndefinedBehaviorSanitizer for debug runs.
- `clang-tidy` and `cppcheck` when configured; do not add new findings.
- No `assert()`-based runtime validation.
- No new non-static symbol without a declaration and a reason to export it.

## 14. Testing and Validation

Every change must at least keep the existing build targets clean:

```bash
make            # release build
make dev        # debug build
make leaks      # sanitizer build
./bin/vlsx-leaks
```

`make leaks` builds `bin/vlsx-leaks` with AddressSanitizer and
UndefinedBehaviorSanitizer; run the binary and exercise the affected screen,
not just the compile step.

As the `core/` layer is extracted, add and use:

```bash
make test        # unit tests for core/common, no termbox2
make check-layers
```

Tests live in `tests/` and must link without `termbox2.h`, `app.c`, `ui.c`,
`render.c`, or any screen module. New calculation behavior requires a
regression test where the test target exists, including IPv4 and VLSM boundary
values, invalid input, and capacity failures.

TUI changes require a manual check of the affected screens:

- terminal smaller than 90x24 shows the size error;
- sidebar, tabbar, and input focus transitions work, including `ESC`;
- valid and invalid IPv4 input behave correctly;
- valid and invalid VLSM input behave correctly;
- VLSM table scrolling works with arrow keys and `h`/`j`/`k`/`l`;
- an invalid input after a successful calculation does not leave stale results
  on the screen;
- resizing the terminal redraws without artifacts.

### 14.1. Definition of done

Before a pull request is ready:

1. The file compiles with the target warning flags without a single warning.
2. Every non-exported function and global is `static`.
3. Every resource is released through the cleanup path on every error branch.
4. Core tests pass under ASan/UBSan without leaks.
5. No `termbox2.h` or `tb_*` appears outside `tui/`.
6. `core/` and `common/` do not include or touch `app`, screens, or TUI code.
7. Invalid input does not leave stale results or validity flags behind.

## 15. Commits

- Use Conventional Commits: `feat:`, `fix:`, `refactor:`, `style:`, `docs:`,
  `test:`, `build:`, or `chore:`.
- Scope commits by module when it helps: `feat(vlsm):`, `refactor(ipv4):`,
  `fix(events):`.
- Keep commits focused. Formatting-only changes belong in a `style:` commit.
- Do not mix a file move, a behavior change, and a formatting pass in one
  commit.
- Write commit messages in English and describe the intent, not just the files
  touched.
- Sign commits when the repository or your environment requires it; never
  bypass signing or hook checks.
