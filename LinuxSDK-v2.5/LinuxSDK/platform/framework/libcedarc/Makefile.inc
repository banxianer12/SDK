
CFLAGS_CDCG = -Wall -Werror -D_GNU_SOURCE \
              -Wno-unused-variable \
              -Wno-unused-but-set-variable \
              -Wno-unused-result \
              -Wno-unused-function \
              -Wno-unused-value \
              -Wno-pointer-to-int-cast \
              -Wno-int-to-pointer-cast \
              -Wno-unused-label \
              -Wno-strict-aliasing \
              -Wno-format \
              -Wno-switch-unreachable\
              -lm

CPPFLAGS_CDCG = -Wall -Werror -D_GNU_SOURCE \
                -Wno-unused-variable \
                -Wno-unused-but-set-variable \
                -Wno-unused-value \
                -Wno-unused-result \
                -Wno-format \
                -Wno-switch-unreachable \
                -lm

LDFLAGS_CDCG = -Wl,--no-undefined -avoid-version

