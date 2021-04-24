
# sdbusout::=$(addsuffix .out,$(addprefix bus,-service -client))
# $(MAKE) $(word 1,$(sdbusout))

# call:
# # 	@echo sleep 1; sleep 1
# 	@busctl --user call         net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Multiply xx  5 7
# 	@busctl --user call         net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Divide   xx 72 2
# 	@busctl --user get-property net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Data
# 	@busctl --user call         net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Multiply xx  5 7
# 	@busctl --user call         net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Divide   xx 72 2
# 	@busctl --user get-property net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Data

# service:
# 	@killall -SIGINT bus_calc_service.out || /bin/true
# 	@$(MAKE) clean
# 	$(MAKE) bus_calc_service.out
# # 	@./bus_calc_service.out
# # 	@valgrind ./bus_calc_service.out
# 	@valgrind -s --leak-check=full --show-leak-kinds=all ./bus_calc_service.out

# bus_calc_service.out:
# bus_%.out: bus_%.c
# 	$(CC) $(CFLAGS) $(shell pkg-config --cflags libsystemd) -o $@ $< $(shell pkg-config --libs libsystemd)
