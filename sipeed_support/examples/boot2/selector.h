typedef struct {
    volatile uint8_t __index;
    const uint8_t __max;
    uint32_t __flags;
} selector_t;
#define SELECTOR_INITED_MAGIC 0xDEADBEEF

static inline selector_t selector_init(uint8_t __max)
{
    selector_t selecetor = {
        .__max = __max,
        .__index = __max - 1,
        .__flags = SELECTOR_INITED_MAGIC,
    };
    return selecetor;
}

static inline void selector_next(selector_t *selector)
{
    assert(SELECTOR_INITED_MAGIC == selector->__flags && selector->__max > 0);
    if (likely(selector->__index + 1 < selector->__max)) {
        selector->__index += 1;
    } else {
        selector->__index = (selector->__index + 1) % selector->__max;
    }
}

static inline uint8_t selector_idx(selector_t *selector)
{
    assert(SELECTOR_INITED_MAGIC == selector->__flags && selector->__max > 0);
    return selector->__index;
}