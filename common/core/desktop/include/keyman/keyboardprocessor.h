/*
  Copyright:    © 2018 SIL International.
  Description:  Cross platform API C/C++ declarations for libkmnkbp keyboard
                processor.
  Create Date:  2 Oct 2018
  Authors:      Tim Eves (TSE)
  History:      18 Oct 2018 - TSE - Finialised verion of API.
                 6 Oct 2018 - TSE - Move into keyman folder.

*/
#pragma once
/*
# Keyman Keyboard Processor API

## Requirements
1. Cross platform.
2. Cross language.
3. Facilitate stateless operation of the Engine.
4. Keyboard format agnostic -- support both KMN and future LDML based keyboards.
5. Support querying Engine attributes.
6. Support querying Keyboard attributes.
7. Idempotent


## Design decisions in support of requirements:
- Use C or C99 types and calling convention for the interface, it has the
  broadest language FFI support. [1,2]
- Have client (Platform layer) code load keyboards, manage & pass state. [3,4,7]
- Provide query calls to return static attributes data for keyboards and
  engine [5,6]
- Provide get/set calls for client accessible keyboard state information [3,4]


## Glossary
- __Platform layer:__
The code that consumes the Keyman Keyboard Processor API, and provides the
operating system-specific handling of keystroke events and integration with
applications.
- __Client Application:__
The application that has the focus and receives text events from the Platform
layer.
- __Context:__ Text preceding the insertion point
- __Marker:__ Positional state that can be placed in the Context.
- __Keyboard:__ A set of rules for execution by an Engine
- __Option:__ A variable in a dynamic or static key value store.
- __Processor:__
The component that implements this API and can parse and execute a particular
keyboard.
- __State:__ An object that holds internal state of the Processor for a given
insertion point
- __Action:__
A directive output by the processor detailing how the Platform layer should
transform the Client Application's text buffer. There may be several items
produced by a single keyboard event.
- __Keyboard Event:__
A virtual key event and modifier map received from the Platform layer to be
processed with the state object for this Client application.
- __Virtual Key:__
A code based on the US English layout, with values matching the Windows
virtual key codes. See `keyboardprocessor_vkeys.h` for definitions.
- __Modifier Key:__
The set of Control, Shift, Alt, Caps Lock keys. On some platforms these may
have other names (e.g. Alt is called Option on macOS); other platform-specific
modifiers such as Windows key are excluded from this set. Some modifiers are
transient, such as Control, and others have long-lasting state, such as
Caps Lock.

## API
### Namespace
All calls, types and enums are prefixed with the namespace identifier `km_kbp_`

### API idioms
Almost all calls marshalling variable length aggregate data in or out of an API
object take the form:
> km_kbp_status *fn_name*(object_ref, buffer_ptr, size_ptr)

where the buffer is nullable and all other arguments are required (will result
in an `KM_KBP_STATUS_INVALID_ARGUMENT` status being returned if nulled). When
`buffer` is `nullptr` or `0` the function will place the size of the required
buffer in the variable pointed to by `size_ptr`.

Calls which result in the allocation of resources, regardless of resulting
ownership, are of the form:
> km_kbp_status *fn_name*(object_ref, out_ptr)

where `out_ptr` is a valid pointer to a caller allocated variable to hold the
resulting ouput. This is often a reference to a created object. All arguments
are required (will result in an `KM_KBP_STATUS_INVALID_ARGUMENT` status being
returned if nulled).

For accessors to fixed size attributes of an object these will take the form:
> attr_value __fn_name__(object_ref)

`object_ref` is required to be valid and will result in a nonsense value being
returned if `nullptr` or `0`.

All dispose calls are designed to accept null as a valid value and will do
nothing in that event.
```c
*/
#include <stdint.h>
#include <stdlib.h>
#include <keyman/keyboardprocessor_bits.h>
#include <keyman/keyboardprocessor_vkeys.h>
#include <keyman/keyboardprocessor_version.h>

#if defined(__cplusplus)
extern "C"
{
#endif
// Basic types
//
#if defined(__cplusplus)
typedef char16_t   km_kbp_cp;
typedef char32_t   km_kbp_usv;
#else
typedef uint16_t    km_kbp_cp;          // code point
typedef uint32_t    km_kbp_usv;         // Unicode Scalar Value
#endif
typedef uint16_t    km_kbp_virtual_key; // A virtual key code.
typedef uint32_t    km_kbp_status;      // Status return code.

// Opaque object types.
//
typedef struct km_kbp_context     km_kbp_context;
typedef struct km_kbp_keyboard    km_kbp_keyboard;
typedef struct km_kbp_state       km_kbp_state;
typedef struct km_kbp_options     km_kbp_options;

// Forward declarations
//
typedef struct km_kbp_option_item  km_kbp_option_item;

/*```
### Error Handling
Error handling and success failure notification are communicated through a
general mechanism similar to COM’s `HRESULT` scheme (unlike COM, any non-zero
value is an error). Any functions that can fail will always return a status
value and all results are returned via outparams passed to the function.
```c
*/
enum km_kbp_status_codes {
  KM_KBP_STATUS_OK = 0,
  KM_KBP_STATUS_NO_MEM = 1,
  KM_KBP_STATUS_IO_ERROR = 2,
  KM_KBP_STATUS_INVALID_ARGUMENT = 3,
  KM_KBP_STATUS_KEY_ERROR = 4,
  KM_KBP_STATUS_INSUFFICENT_BUFFER = 5,
  KM_KBP_STATUS_INVALID_UTF = 6,
  KM_KBP_STATUS_INVALID_KEYBOARD = 7,
  KM_KBP_STATUS_OS_ERROR = 0x80000000
};

/*
```
The final status code KM_KBP_STATUS_OS_ERROR is intended to allow encapsulating
a platform error code; the remaining 31 low bits are the error code returned by
the OS for cases where the failure mode is platform specific. For HRESULT codes
this only permits failure codes to be passed.


### Context
The context is the text prior to the insertion point (caret, cursor).
The context is constructed by the Platform layer, typically by interrogating the
Client Application.  The context will be updated by the engine for keystroke
events.  If the Platform layer code caches the context, the context should be
reset when a context state change is detected. Context state changes can occur
when the user uses the mouse to move the insertion point, uses cursor keys,
switches applications or input fields, or presses hotkeys such as Ctrl+N to
start a new document. The full set of context state change triggers is up to the
Platform layer.

Context can also contain positional Markers (also known as 'deadkeys' in kmn
keyboards), which are transitory state flags that are erased whenever a context
state change is detected. Markers are always controlled by the Engine.

Contexts are always owned by their state.  They may be set to a list of
context_items or interrogated for their current list of context items.
```c
*/
enum km_kbp_context_type {
  KM_KBP_CT_END,
  KM_KBP_CT_CHAR,
  KM_KBP_CT_MARKER
};

typedef struct {
  uint8_t   type;
  uint8_t   _reserved[3];
  union {
    km_kbp_usv  character;
    uint32_t    marker;
  };
} km_kbp_context_item;

#define KM_KBP_CONTEXT_ITEM_END {KM_KBP_CT_END, {0,}, {0,}}
/*
```
### `km_kbp_context_items_from_utf16`
##### Description:
Convert a UTF16 encoded Unicode string into an array of `km_kbp_context_item`
structures. Allocates memory as needed.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_INVALID_ARGUMENT`: If non-optional parameters are null.
- `KM_KBP_STATUS_NO_MEM`: In the event not enough memory can be allocated for the
  output buffer.
- `KM_KBP_STATUS_INVALID_UTF`: In the event the UTF16 string cannot be decoded
  because it contains unpaired surrogate codeunits.
##### Parameters:
- __text__: a pointer to a null terminated array of utf16 encoded data.
- __out_ptr__: a pointer to the result variable:
    A pointer to the start of the `km_kbp_context_item` array containing the
    representation of the input string.
    Terminated with a type of `KM_KBP_CT_END`. Must be disposed of with
    `km_kbp_context_items_dispose`.

```c
*/
KMN_API
km_kbp_status
km_kbp_context_items_from_utf16(km_kbp_cp const *text,
                                km_kbp_context_item **out_ptr);

/*
```
### `km_kbp_context_items_from_utf8`
##### Description:
Convert an UTF8 encoded Unicode string into an array of `km_kbp_context_item`
structures. Allocates memory as needed.
##### Status:
- `KM_KBP_STATUS_INVALID_ARGUMENT`: If non-optional parameters are null.
- `KM_KBP_STATUS_NO_MEM`: In the event it cannot allocate enough memory for the
  output buffer.
- `KM_KBP_STATUS_INVALID_UTF`: In the event the UTF8 string cannot be
decoded.
##### Parameters:
- __text__: a pointer to a null terminated array of utf8 encoded data.
- __out_ptr__: a pointer to the result variable:
    A pointer to the  start of the `km_kbp_context_item` array containing the
    representation of the input string.
    Terminated with a type of `KM_KBP_CT_END`.

```c
*/
KMN_API
km_kbp_status
km_kbp_context_items_from_utf8(char const *text,
                                km_kbp_context_item **out_ptr);

/*
```
### `km_kbp_context_items_to_utf16`
##### Description:
Convert a context item array into a UTF-16 encoded string placing it into
the supplied buffer of specified size, and return the number of code units
actually used in the conversion. If null is passed as the buffer the
number codeunits required is returned. This will strip markers from the
context during the conversion.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_INVALID_ARGUMENT`: If non-optional parameters are null.
- `KM_KBP_STATUS_INSUFFICENT_BUFFER`: If the buffer is not large enough.
  `buf_size` will contain the space required. The contents of the buffer are
  undefined.
##### Parameters:
- __context_items__: A pointer to the start of an array `km_kbp_context_item`.
    Must be terminated with a type of `KM_KBP_CT_END`.
- __buf__: A pointer to the buffer to place the UTF-16 string into.
    May be null to request size calculation.
- __buf_size__: a pointer to the result variable:
    The size of the supplied buffer in codeunits if `buf` is given.
    On return will be the size required if `buf` is null.

```c
*/
KMN_API
km_kbp_status
km_kbp_context_items_to_utf16(km_kbp_context_item const *item,
                              km_kbp_cp *buf,
                              size_t *buf_size);

/*
```
### `km_kbp_context_items_to_utf8`
##### Description:
Convert a context item array into a UTF-8 encoded string placing it into
the supplied buffer of specified size, and return the number of code units
actually used in the conversion. If null is passed as the buffer the
number codeunits required is returned. This will strip markers from the
context during the conversion.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_INVALID_ARGUMENT`: If non-optional parameters are null.
- `KM_KBP_STATUS_INSUFFICENT_BUFFER`: If the buffer is not large enough.
  `buf_size` will contain the space required. The contents of the buffer are
  undefined.
##### Parameters:
- __context_items__: A pointer to the start of an array `km_kbp_context_item`.
    Must be terminated with a type of `KM_KBP_CT_END`.
- __buf__: A pointer to the buffer to place the UTF-8 string into.
    May be null to request size calculation.
- __buf_size__: a pointer to the result variable:
    The size of the supplied buffer in codeunits if `buf` is given.
    On return will be the size required if `buf` is null.

```c
*/
KMN_API
km_kbp_status
km_kbp_context_items_to_utf8(km_kbp_context_item const *item,
                              char *buf,
                              size_t *buf_size);

/*
```
### `km_kbp_context_items_dispose`
##### Description:
Free the allocated memory belonging to a `km_kbp_context_item` array previously
returned by `km_kbp_context_items_from_utf16` or `km_kbp_context_get`
##### Parameters:
- __context_items__: A pointer to the start of the `km_kbp_context_item` array
    to be disposed of.

```c
*/
KMN_API
void
km_kbp_context_items_dispose(km_kbp_context_item *context_items);

/*
```
### `km_kbp_context_set`
##### Description:
Replace the contents of the current context with a new sequence of
`km_kbp_context_item` entries.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_INVALID_ARGUMENT`: If non-optional parameters are null.
- `KM_KBP_STATUS_NO_MEM`: In the event not enough memory can be allocated to
  grow the context buffer internally.
##### Parameters:
- __context__: A pointer to an opaque context object
- __context_items__: A pointer to the start of the `km_kbp_context_item`
    array containing the new context. It must be terminated with an item
    of type `KM_KBP_CT_END`.

```c
*/
KMN_API
km_kbp_status
km_kbp_context_set(km_kbp_context *context,
                   km_kbp_context_item const *context_items);

/*
```
### `km_kbp_context_get`
##### Description:
Copies all items in the context into a new array and returns the new array.
This must be disposed of by caller using `km_kbp_context_items_dispose`.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_INVALID_ARGUMENT`: If non-optional parameters are null.
- `KM_KBP_STATUS_NO_MEM`: In the event not enough memory can be allocated for the
  output buffer.
##### Parameters:
- __context_items__: A pointer to the start of an array `km_kbp_context_item`.
- __out__: a pointer to the result variable:
    A pointer to the start of the `km_kbp_context_item` array containing a
    copy of the context. Terminated with a type of `KM_KBP_CT_END`. Must be
    disposed of with `km_kbp_context_items_dispose`.

```c
*/
KMN_API
km_kbp_status
km_kbp_context_get(km_kbp_context const *context_items,
                   km_kbp_context_item **out);

/*
```
### `km_kbp_context_clear`
##### Description:
Removes all context_items from the internal array. If `context` is
null, has no effect.
##### Parameters:
- __context__: A pointer to an opaque context object

```c
*/
KMN_API
void
km_kbp_context_clear(km_kbp_context *);

/*
```
### `km_kbp_context_length`
##### Description:
Return the number of items in the context.
##### Return:
The number of items in the context, and will return 0 if passed a null `context`
pointer.
##### Parameters:
- __context__: A pointer to an opaque context object

```c
*/
KMN_API
size_t
km_kbp_context_length(km_kbp_context *);

/*
```
### `km_kbp_context_append`
##### Description:
Add more items to the end (insertion point) of the context. If these exceed the
maximum context length the same number of items will be dropped from the
beginning of the context.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_INVALID_ARGUMENT`: If non-optional parameters are null.
- `KM_KBP_STATUS_NO_MEM`: In the event not enough memory can be allocated to
  grow the context buffer internally.
##### Parameters:
- __context__: A pointer to an opaque context object.
- __context_items__: A pointer to the start of the `KM_KBP_CT_END` terminated
    array of `km_kbp_context_item` to append.

```c
*/
KMN_API
km_kbp_status
km_kbp_context_append(km_kbp_context *context,
                      km_kbp_context_item const *context_items);

/*
```
### `km_kbp_context_shrink`
##### Description:
Remove a specified number of items from the end of the context, optionally
add up to the same number of the supplied items to the front of the context.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_INVALID_ARGUMENT`: If non-optional parameters are null.
- `KM_KBP_STATUS_NO_MEM`: in the event it cannot allocated enough memory to grow
  the context internally.
##### Parameters:
- __context__: A pointer to an opaque context object.
- __num__: The number of items to remove from the end of context.
- __context_items__: Pointer to the start of the `KM_KBP_CT_END` terminated
    array of `km_kbp_context_item` to add to the front. Up to `num` items will
    be prepended. This may be null if not required.

```c
*/
KMN_API
km_kbp_status
km_kbp_context_shrink(km_kbp_context *context,
                      size_t num,
                      km_kbp_context_item const *prefix);

/*
```
### Action Items
These provide the results of processing a key event to the Platform layer and
should be processed by the Platform layer to issue commands to the os text
services framework to transform the text store in the Client Application, among
other actions.
```c
*/

typedef struct {
  uint8_t expected_type;     // km_kbp_backspace_type
  uintptr_t expected_value;  // used mainly in unit tests
} km_kbp_backspace_item;

enum km_kbp_backspace_type {
  KM_KBP_BT_UNKNOWN    = 0,  // Used at beginning of context; user-initiated backspace
  KM_KBP_BT_CHAR       = 1,  // Deleting a character prior to insertion point
  KM_KBP_BT_MARKER     = 2,  // Deleting a marker prior to insertion point
  KM_KBP_BT_MAX_TYPE_ID
};

typedef struct {
  uint8_t   type;
  uint8_t   _reserved[sizeof(void*)-sizeof(uint8_t)];
  union {
    uintptr_t             marker;          // MARKER type
    km_kbp_option_item    const * option;  // OPT types
    km_kbp_usv            character;       // CHAR type
    uint8_t               capsLock;        // CAPSLOCK type, 1 to turn on, 0 to turn off
    km_kbp_backspace_item backspace;       // BACKSPACE type
  };
} km_kbp_action_item;

enum km_kbp_action_type {
  KM_KBP_IT_END         = 0,  // Marks end of action items list.
  KM_KBP_IT_CHAR        = 1,  // A Unicode character has been generated.
  KM_KBP_IT_MARKER      = 2,  // Correlates to kmn's "deadkey" markers.
  KM_KBP_IT_ALERT       = 3,  // The keyboard has triggered a alert/beep/bell.
  KM_KBP_IT_BACK        = 4,  // Delete the codepoint preceding the insertion point.
  KM_KBP_IT_PERSIST_OPT = 5,  // The indicated option needs to be stored.
  KM_KBP_IT_EMIT_KEYSTROKE = 6,  // Emit the current keystroke to the application
  KM_KBP_IT_INVALIDATE_CONTEXT = 7,
          // The processor requests that the context buffer be cleared;
          // for applications where context is cached, this clears the context;
          // for applications where context is read from the focused text store,
          // the context is just re-read and markers flushed.
  KM_KBP_IT_CAPSLOCK    = 8,  // Enable or disable capsLock
  KM_KBP_IT_MAX_TYPE_ID
};


/*
```
### Options
A state’s default options are set from the keyboard at creation time and the
environment. The Platform layer is then is expected to apply any persisted
options it is maintaining.  Options are passed into and out of API functions as
simple C arrays of `km_kbp_option_item` terminated with a `KM_KBP_OPTIONS_END`
sentinel value. A state's options are exposed and manipulatable via the
`km_kbp_options` API. All option values are of type C string.

During processing when the Platform layer finds a PERSIST action type it should
store the updated option in the appropriate place, based on its scope.
For RESET the processor will apply the pristine value from the original scope,
the Platform layer should update that only if it manages a previously persisted
value.
```c
*/

enum km_kbp_option_scope {
  KM_KBP_OPT_UNKNOWN      = 0,
  KM_KBP_OPT_KEYBOARD     = 1,
  KM_KBP_OPT_ENVIRONMENT  = 2,
  KM_KBP_OPT_MAX_SCOPES
};

struct km_kbp_option_item {
  km_kbp_cp const *   key;
  km_kbp_cp const *   value;
  uint8_t             scope;  // Scope which an option belongs to.
};

#define KM_KBP_OPTIONS_END { 0, 0, 0 }


/*
```
### `km_kbp_options_list_size`
##### Description:
Return the length of a terminated `km_kbp_option_item` array (options
list).
##### Return:
The number of items in the list or 0 if `opts` is null.
##### Parameters:
- __opts__: A pointer to a `KM_KBP_OPTIONS_END` terminated array of
    `km_kbp_option_item` values.

```c
*/
KMN_API
size_t
km_kbp_options_list_size(km_kbp_option_item const *opts);

/*
```
### `km_kbp_state_option_lookup`
##### Description:
Lookup an option based on its key, in an options list.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_INVALID_ARGUMENT`: If non-optional parameters are null, or
  if the scope is invalid.
- `KM_KBP_STATUS_KEY_ERROR`: The key cannot be found.
##### Parameters:
- __state__: An opaque pointer to a state object.
- __scope__: Which key-value store to interrogate.
- __key__: A UTF-16 string that matches the key in the target `km_kbp_option_item`.
- __value__: A pointer to the result variable:
  A pointer to a UTF-16 string value owned by the state or keyboard object at
  the time of the call. This pointer is only valid *until* the next call to any
  function on this API and should be used immediately.
```c
*/
KMN_API
km_kbp_status
km_kbp_state_option_lookup(km_kbp_state const *state,
                      uint8_t scope,
                      km_kbp_cp const *key,
                      km_kbp_cp const **value);

/*
```
### `km_kbp_state_options_update`
##### Description:
Adds or updates one or more options from a list of `km_kbp_option_item`s.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_INVALID_ARGUMENT`: If non-optional parameters are null.
- `KM_KBP_STATUS_NO_MEM`: In the event an internal memory allocation fails.
- `KM_KBP_STATUS_KEY_ERROR`: The key cannot be found.
##### Parameters:
- __state__: An opaque pointer to a state object.
- __new_opts__: An array of `km_kbp_option_item` objects to update or add. Must be
    terminated with `KM_KBP_OPTIONS_END`.

```c
*/
KMN_API
km_kbp_status
km_kbp_state_options_update(km_kbp_state *state,
                      km_kbp_option_item const *new_opts);

/*
```
### `km_kbp_state_options_to_json`
##### Description:
Export the contents of a `km_kbp_options` array to a JSON formatted document and
place it in the supplied buffer, reporting how much space was used. If null is
passed as the buffer the number of bytes required is returned in `space`. If
there is insufficent space to hold the document the contents of the buffer is
undefined. The returned buffer uses UTF-8 encoding.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_INVALID_ARGUMENT`: If non-optional parameters are null.
- `KM_KBP_STATUS_NO_MEM`: In the event an internal memory allocation fails.
##### Parameters:
- __opts__: An opaque pointer to a state object.
- __buf__: A pointer to the buffer to place the C string containing the JSON
document into, can be null.
- __space__: A pointer to a size_t variable. This variable must contain the
number of bytes available in the buffer pointed to by `buf`, unless `buf` is
null. On return it will hold how many bytes were used.

```c
*/
KMN_API
km_kbp_status
km_kbp_state_options_to_json(km_kbp_state const *state,
                       char *buf,
                       size_t *space);


/*
```
### Keyboards
A keyboard is a set of rules and transforms in a Processor specific format for
transforming key events into action items. The keyboard is parsed and loaded by
the processsor and made available in an immutable fashion for use with any number
of state objects.
```c
*/
typedef struct {
  km_kbp_cp const * version_string;   // Processor specific version string.
  km_kbp_cp const * id;               // Keyman keyboard ID string.
  km_kbp_path_name  folder_path;      // Path to the unpacked folder containing
                                      // the keyboard and associated resources.
  km_kbp_option_item const * default_options;
} km_kbp_keyboard_attrs;

/*
```
### `km_kbp_keyboard_load`
##### Description:
Parse and load keyboard from the supplied path and a pointer to the loaded keyboard
into the out paramter.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_NO_MEM`: In the event an internal memory allocation fails.
- `KM_KBP_STATUS_IO_ERROR`:
    In the event the keyboard file is unparseable for any reason
- `KM_KBP_STATUS_INVALID_ARGUMENT`:
    In the event the file doesn't exist or is inaccesible or `keyboard` is null.
- `KM_KBP_STATUS_OS_ERROR`: Bit 31 (high bit) set, bits 0-30 are an OS-specific
    error code.
##### Parameters:
- __kb_path__: On Windows, a UTF-16 string; on other platforms, a C string:
    contains a valid path to the keyboard file.
- __keyboard__: A pointer to result variable:
    A pointer to the opaque keyboard object returned by the Processor. This
    memory must be freed with a call to `km_kbp_keyboard_dispose`.

```c
*/
KMN_API
km_kbp_status
km_kbp_keyboard_load(km_kbp_path_name kb_path,
                     km_kbp_keyboard **keyboard);

/*
```
### `km_kbp_keyboard_dispose`
##### Description:
Free the allocated memory belonging to an opaque keyboard object previously
returned by `km_kbp_keyboard_load`.
##### Parameters:
- __keyboard__: A pointer to the opaque keyboard object to be
    disposed of.

```c
*/
KMN_API
void
km_kbp_keyboard_dispose(km_kbp_keyboard *keyboard);

/*
```
### `km_kbp_keyboard_get_attrs`
##### Description:
Returns the const internal attributes of the keyboard. This structure is valid
for the lifetime of the opaque keyboard object. Do not modify the returned data.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_INVALID_ARGUMENT`: If non-optional parameters are null.
##### Parameters:
- __keyboard__: A pointer to the opaque keyboard object to be queried.
- __out__: A pointer to the result:
    A pointer to a `km_kbp_keyboard_attrs` structure.

```c
*/
KMN_API
km_kbp_status
km_kbp_keyboard_get_attrs(km_kbp_keyboard const *keyboard,
                          km_kbp_keyboard_attrs const **out);

/*
```
### State
A State object maintains all per keyboard related state including context
and dynamic options ("option stores" in kmn format).

```c
*/

/*
```
### `km_kbp_state_create`
##### Description:
Create a keyboard processor state object, maintaining state for the keyboard in
the environment passed.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_NO_MEM`:
  In the event memory is unavailable to allocate a state object.
- `KM_KBP_STATUS_INVALID_ARGUMENT`:
  In the event the `keyboard` or `out` pointer are null.
##### Parameters:
- __keyboard__:
A pointer to the opaque keyboard object this object will hold state for.
- __env__:
The array of `km_kbp_option_item` key/value pairs used to initialise the
environment, terminated by `KM_KBP_OPTIONS_END`.
- __out__:
A pointer to result variable: A pointer to the opaque state object
returned by the Processor, initalised to maintain state for `keyboard`.
This must be disposed of by a call to `km_kbp_state_dispose`.

```c
*/
KMN_API
km_kbp_status
km_kbp_state_create(km_kbp_keyboard *keyboard,
                    km_kbp_option_item const *env,
                    km_kbp_state **out);

/*
```
### `km_kbp_state_clone`
##### Description:
Clone an existing opaque state object.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_NO_MEM`:
In the event memory is unavailable to allocate a state object.
- `KM_KBP_STATUS_INVALID_ARGUMENT`:
In the event the `state` or `out` pointer are null.
##### Parameters:
- __state__:
A pointer to the opaque statea object to be cloned.
- __out__:
A pointer to result variable: A pointer to the opaque state object
returned by the Processor, cloned from the existing object `state`. This
must be disposed of by a call to `km_kbp_state_dispose`.

```c
*/
KMN_API
km_kbp_status
km_kbp_state_clone(km_kbp_state const *state,
                   km_kbp_state **out);

/*
```
### `km_kbp_state_dispose`
##### Description:
Free the allocated resources belonging to a `km_kbp_state` object previously
returned by `km_kbp_state_create` or `km_kbp_state_clone`. After this all
pointers previously returned by any km_kbp_state family of calls will become
invalid.
##### Parameters:
- __state__: A pointer to the opaque state object to be disposed.

```c
*/
KMN_API
void
km_kbp_state_dispose(km_kbp_state *state);

/*
```
### `km_kbp_state_context`
##### Description:
Get access to the state object's context.
##### Return:
A pointer to an opaque state object. This pointer is valid for the lifetime
of the state object. If null is passed in, then null is returned.
##### Parameters:
- __state__: A pointer to the opaque state object to be queried.

```c
*/
KMN_API
km_kbp_context *
km_kbp_state_context(km_kbp_state *state);

/*
```
### `km_kbp_state_action_items`
##### Description:
Get the list of action items generated by the last call to
`km_kbp_process_event`.
##### Return:
A pointer to a `km_kbp_action_item` list, of `*num_items` in length. This data
becomes invalid when the state object is destroyed, or after a call to
`km_kbp_process_event`. Do not modify the contents of this data. The returned
array is terminated with a `KM_KBP_IT_END` entry.
##### Parameters:
- __state__: A pointer to the opaque `km_kbp_state` object to be queried.
- __num_items__:
A pointer to a result variable: The number of items in the action item list
including the `KM_KBP_IT_END` terminator. May be null if not that
information is required.

```c
*/
KMN_API
km_kbp_action_item const *
km_kbp_state_action_items(km_kbp_state const *state,
                          size_t *num_items);

/*
```
### `km_kpb_state_to_json`
##### Description:
Export the internal state of a `km_kbp_state` object to a JSON format document
and place it in the supplied buffer, reporting how much space was used. If null
is passed as the buffer the number of bytes required is returned. If there is
insufficent space to hold the document, the contents of the buffer is undefined.
The encoding of the returned data is UTF-8.

__WARNING__: The structure and format of the JSON document while independently
versioned is not part of this API and is intended solely for use in diagnostics
or by development and debugging tools which are aware of processor
implementation details.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_NO_MEM`: In the event an internal memory allocation fails.
##### Parameters:
- __state__: An pointer to an opaque state object.
- __buf__: A pointer to the buffer to place the C string containing the JSON
document into. May be null.
- __space__: A pointer to a size_t variable. This variable must contain the
number of bytes available in the buffer pointed to by `buf`, unless `buf` is
null. On return it will hold how many bytes were used.

```c
*/
KMN_API
km_kbp_status
km_kbp_state_to_json(km_kbp_state const *state,
                     char *buf,
                     size_t *space);

/*
```
### Processor
```c
*/
typedef struct {
  size_t      max_context;  // Maximum context size supported by processor.
  uint16_t    current;      // Current API number supported.
  uint16_t    revision;     // Implementation number of current API.
  uint16_t    age;          // current - age == Oldest API number supported.
  uint16_t    technology;   // A bit field specifiying which Keyboard
                            //  technologies the engine supports.
  char const *vendor;       // Implementor of the processor.
} km_kbp_attr;

enum km_kbp_tech_value {
  KM_KBP_TECH_UNSPECIFIED = 0,
  KM_KBP_TECH_MOCK        = 1 << 0,
  KM_KBP_TECH_KMX         = 1 << 1,
  KM_KBP_TECH_LDML        = 1 << 2,
  KM_KBP_TECH_RUST_MOCK   = 1 << 3
};


/*
```
### `km_kbp_get_engine_attrs`
##### Description:
Get access processors attributes describing version and technology implemented.
##### Return:
A pointer to a `km_kbp_attr` structure. Do not modify the contents of this
structure.
##### Parameters:
- __state__: An opaque pointer to an `km_kbp_state`.
```c
*/
KMN_API
km_kbp_attr const *
km_kbp_get_engine_attrs(km_kbp_state const *state);

/*
```
### `km_kbp_process_event`
##### Description:
Run the keyboard on an opaque state object with the provided virtual key and modifer
key state. Updates the state object as appropriate and fills out its action list.

The action list will be cleared at the start of this call; options and context in
the state may also be modified.
##### Return status:
- `KM_KBP_STATUS_OK`: On success.
- `KM_KBP_STATUS_NO_MEM`:
In the event memory is unavailable to allocate internal buffers.
- `KM_KBP_STATUS_INVALID_ARGUMENT`:
In the event the `state` pointer is null or an invalid virtual key or modifier
state is passed.

##### Parameters:
- __state__: A pointer to the opaque state object.
- __vk__: A virtual key to be processed.
- __modifier_state__:
The combinations of modifier keys set at the time key `vk` was pressed, bitmask
from the `km_kbp_modifier_state` enum.

```c
*/
KMN_API
km_kbp_status
km_kbp_process_event(km_kbp_state *state,
                     km_kbp_virtual_key vk,
                     uint16_t modifier_state,
                     uint8_t is_key_down);

#if defined(__cplusplus)
} // extern "C"
#endif
/*```
*/
