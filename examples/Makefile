
.PHONY: all clean cleanall

all:
	@$(MAKE) all -C ldap
	@$(MAKE) all -C goose
	@$(MAKE) all -C sv
	@$(MAKE) all -C mms
	@$(MAKE) all -C krb5

clean:
	@$(MAKE) clean -C ldap
	@$(MAKE) clean -C goose
	@$(MAKE) clean -C sv
	@$(MAKE) clean -C mms
	@$(MAKE) clean -C krb5

cleanall:
	@$(MAKE) cleanall -C ldap
	@$(MAKE) cleanall -C goose
	@$(MAKE) cleanall -C sv
	@$(MAKE) cleanall -C mms
	@$(MAKE) cleanall -C krb5
