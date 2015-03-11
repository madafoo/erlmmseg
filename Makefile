REBAR=rebar

all: compile

compile:
	@$(REBAR) compile

clean:
	@$(REBAR) clean

test:
	@$(REBAR) eunit skip_deps=true

dialyzer:
	@dialyzer -Wrace_conditions -Wunderspecs -n ebin

doc:
	@$(REBAR) doc skip_deps=true

.PHONY: test doc
