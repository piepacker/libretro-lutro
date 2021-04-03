#ifndef LUTRO_ASSERT_H
#define LUTRO_ASSERT_H

// setup some sane defaults for assertions - these should ideally be specified explicitly by the build system.

#if !defined(LUTRO_ENABLE_PLAYER_ASSERT)
#  define LUTRO_ENABLE_PLAYER_ASSERT  1
#endif

#if !defined(LUTRO_ENABLE_TOOL_ASSERT)
#  define LUTRO_ENABLE_TOOL_ASSERT    1
#endif

#if !defined(LUTRO_ENABLE_DBG_ASSERT)
#  if defined(NDEBUG)
#    define LUTRO_ENABLE_DBG_ASSERT   0
#  else
#    define LUTRO_ENABLE_DBG_ASSERT   1
#  endif
#endif

// Some macro engineering notes:
//  - the disabled versions of assertions still include the cond, gated behind `0 &&` which makes the cond unreachable.
//    this is done to enforce syntax checking of the contents of the assertion conditional even in release builds.

extern int _lutro_assertf_internal(int ignorable, const char *fmt, ...);

#define _base_hard_error()                            ((void)(          ((_lutro_assertf_internal(0, __FILE__ "(%d):unrecoverable error"         "\n", __LINE__                        ),1) && (           (abort(), 0)))))
#define _base_hard_errorf(msg, ...)                   ((void)(          ((_lutro_assertf_internal(0, __FILE__ "(%d):unrecoverable error "    msg "\n", __LINE__,         ## __VA_ARGS__),1) && (           (abort(), 0)))))
#define _base_soft_error(doAbort)                     ((void)(          ((_lutro_assertf_internal(1, __FILE__ "(%d):error"                       "\n", __LINE__                        )  ) && (doAbort && (abort(), 0)))))
#define _base_soft_errorf(doAbort, msg, ...)          ((void)(          ((_lutro_assertf_internal(1, __FILE__ "(%d):error "                  msg "\n", __LINE__,         ## __VA_ARGS__)  ) && (doAbort && (abort(), 0)))))

#define _base_hard_assert(cond)                       ((void)((cond) || ((_lutro_assertf_internal(0, __FILE__ "(%d):assertion `%s` failed."      "\n", __LINE__, #cond                 ),1) && (           (abort(), 0)))))
#define _base_hard_assertf(cond, msg, ...)            ((void)((cond) || ((_lutro_assertf_internal(0, __FILE__ "(%d):assertion `%s` failed. " msg "\n", __LINE__, #cond , ## __VA_ARGS__),1) && (           (abort(), 0)))))
#define _base_soft_assert(doAbort, cond)              ((void)((cond) || ((_lutro_assertf_internal(1, __FILE__ "(%d):assertion `%s` failed."      "\n", __LINE__, #cond                 )  ) && (doAbort && (abort(), 0)))))
#define _base_soft_assertf(doAbort, cond, msg, ...)   ((void)((cond) || ((_lutro_assertf_internal(1, __FILE__ "(%d):assertion `%s` failed. " msg "\n", __LINE__, #cond , ## __VA_ARGS__)  ) && (doAbort && (abort(), 0)))))

// Soft vs Hard Assertions
//  - Soft assertions are ignorable: a message is displayed to the user and a crash report can optionally be
//    uploaded to some centralized portal for content creators. Hitting ignore allows the engine to continue
//    ruunning, tho whatever action that asserted will not be applied (typically meaning some audio or visual
//    effect will not function).
// 
//  - Hard Assertions (assumptions) are not ignorable. They are no-return when condition failed, and will end
//    in crash reports and process aborted. These type of asserts should be used sparingly, as they prevent the
//    gameplay content creators from fixing and hit-reloading lua. These are demoted to compiler assumption
//    directives in Player builds. Intended use would be bounds checking or null pointer checks.

// play_assert is always enabled even on Player (standalone) builds of the engine. The majority of assertions
// should assume play_assert first and then select tool_assert or dbg_assert only if performance profiling or
// some other edge case reasioning merits it.
//   - play_assert on Player builds automatically behaves as an 'ignored assert': the assertion is logged and
//     execution is allowed to continue.
//   - play_assert on Tool/Debug builds behaves the same as tool_assert.

#if LUTRO_ENABLE_PLAYER_ASSERT && LUTRO_ENABLE_TOOL_ASSERT
#  define play_assert(cond)                  _base_soft_assert (1, cond)
#  define play_assertf(cond, msg, ...)       _base_soft_assertf(1, cond, msg, ## __VA_ARGS__)
#  define hard_assert(cond)                  _base_hard_assert (   cond)
#  define hard_assertf(cond, msg, ...)       _base_hard_assertf(   cond, msg, ## __VA_ARGS__)
#elif LUTRO_ENABLE_PLAYER_ASSERT
#  define play_assert(cond)                  _base_soft_assert (0, cond)
#  define play_assertf(cond, msg, ...)       _base_soft_assertf(0, cond, msg, ## __VA_ARGS__)
#  define hard_assert(cond)                  _base_hard_assert (   cond)
#  define hard_assertf(cond, msg, ...)       _base_hard_assertf(   cond, msg, ## __VA_ARGS__)
#else
#  define play_assert(cond)             ((void)(0 && (cond)))
#  define play_assertf(cond, msg, ...)  ((void)(0 && (cond)))
#  define hard_assert(cond)             ((void)(0 && (cond)))
#  define hard_assertf(cond, msg, ...)  ((void)(0 && (cond)))
#endif

// tool_assert behaves similarly to play_assert, but is meant to be conditionally compiled out on player
// builds. The main justifications to use tool_assert:
//  - for performance reasons, eg. if a play_assert is known to be a performance issue
//  - to avoid surfacing errors that only make sense from the context of content creation, or simply don't
//    make sense in the context of the player
//
// Currently no 'hard' version of tool assert is provided. Any such hard failure should be able to be 
// performed at a point that is not perf-critical (outside loops) and the value of the things hard
// assertions check for is such that we really want to check for them in Player builds. But also not
// against re-evaluating and adding hard versions if we discover the need for them later on.
#if LUTRO_ENABLE_TOOL_ASSERT
#  define tool_error()                  _base_soft_error  (1)
#  define tool_errorf(msg, ...)         _base_soft_errorf (1, msg, ## __VA_ARGS__)
#  define tool_assert(cond)             _base_soft_assert (1, cond)
#  define tool_assertf(cond, msg, ...)  _base_soft_assertf(1, cond, msg, ## __VA_ARGS__)
#else
#  define tool_assert(cond)             ((void)(0 && (cond)))
#  define tool_assertf(cond, msg, ...)  ((void)(0 && (cond)))
#endif

// dbg_assert is enabled only in debug builds, and is for use only in performance-critical code paths where
// the overhead of the assertion check should be avoided. Use of this macro should be limited only to specific
// situations where a tool_assert is actually observed to be a performance problem, or when it's plainly
// obvious that it might do so (a good example would be assertions within the audio mixer loop).
//
// dbg_assume is an alias for dbg_hard_assert. It might be nice to decide on one or the other later.
#if LUTRO_ENABLE_DBG_ASSERT
#  define dbg_assert(cond)                  _base_soft_assert (1, cond)
#  define dbg_assertf(cond, msg, ...)       _base_soft_assertf(1, cond, msg, ## __VA_ARGS__)
#  define dbg_assume(cond)                  _base_hard_assert (   cond)
#  define dbg_assumef(cond, msg, ...)       _base_hard_assertf(   cond, msg, ## __VA_ARGS__)
#  define dbg_hard_assert(cond)             _base_hard_assert (   cond)
#  define dbg_hard_assertf(cond, msg, ...)  _base_hard_assertf(   cond, msg, ## __VA_ARGS__)
#else
#  define dbg_assert(cond)                  ((void)(0 && (cond)))
#  define dbg_assertf(cond, msg, ...)       ((void)(0 && (cond)))
#  define dbg_assume(cond)                  (__assume(cond))
#  define dbg_assumef(cond, msg, ...)       (__assume(cond))
#  define dbg_hard_assert(cond)             (__assume(cond))
#  define dbg_hard_assertf(cond, msg, ...)  (__assume(cond))
#endif

#endif
