``tee-stats`` is a statistics reporting tool for OP-TEE. It is the client
side of the static Trusted Application `core/arch/arm/sta/stats.c`_ (which
may be enabled by setting CFG_WITH_STATS=y when building OP-TEE).

Currently, it can report information about the TEE kernel heap and optionally
clear the `max_alloc` value. Other statistics exported by `stats.c` may be
added later. Example::

  $ tee-stats -c
  HEAP: cur_alloc 4864 max_alloc 22704 heap_size 106544
  $ tee-stats
  HEAP: cur_alloc 4784 max_alloc 4784 heap_size 106544

.. _core/arch/arm/sta/stats.c: https://github.com/OP-TEE/optee_os/blob/master/core/arch/arm/sta/stats.c
