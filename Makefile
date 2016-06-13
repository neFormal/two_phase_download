RM := rm -rf
LIBS := -lpthread
CPP_SRCS = ../storage.cpp 
OBJS = ./storage.o 
CPP_DEPS = ./storage.d 


%.o: ./%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++1y -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

all: storage

storage: $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross G++ Linker'
	g++  -o "storage" $(OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(OBJS)$(CPP_DEPS) storage
	-@echo ' '

run:
	./storage 4001

.PHONY: all clean dependents
.SECONDARY:
