target_sources(LTL PUBLIC
    ltl.h
    algos.h
    concept.h
    condition.h
    crtp.h
    expected.h
    functional.h
    immutable.h
    invoke.h
    movable_any.h
    operator.h
    optional.h
    optional_type.h
    stream.h
    StrongType.h
    traits.h
    Tuple.h
    tuple_algos.h
    TypedTuple.h
    VariantUtils.h
    fast.h
    coroutine_helpers.h
    thread.h)

add_subdirectory(Range)
