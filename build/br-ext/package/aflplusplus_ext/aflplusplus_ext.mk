AFLPLUSPLUS_EXT_VERSION = 1.0
AFLPLUSPLUS_EXT_SITE = $(BR2_EXTERNAL_OPTEE_PATH)/package/aflplusplus_ext/deploy
AFLPLUSPLUS_EXT_SITE_METHOD = local

define AFLPLUSPLUS_EXT_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/* $(TARGET_DIR)/root
endef

$(eval $(generic-package))