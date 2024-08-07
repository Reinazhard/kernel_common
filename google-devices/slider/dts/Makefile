# SPDX-License-Identifier: GPL-2.0

slider_overlays := \
	google-dts-base/gs101-slider.dtbo \
	google-dts-base/gs101-slider2.dtbo \
	google-dts-base/gs101-slider2-o6r4.dtbo \
	google-dts-base/gs101-whitefin.dtbo \
	google-dts-base/gs101-whitefin2.dtbo \
	google-dts-base/gs101-whitefin2v2.dtbo \
	google-dts-base/gs101-oriole.dtbo \
	google-dts-base/gs101-oriole-evt1_1.dtbo \
	google-dts-base/gs101-oriole-evt-wingboard.dtbo \
	google-dts-base/gs101-oriole-evt1.dtbo \
	google-dts-base/gs101-oriole-proto1.dtbo \
	google-dts-base/gs101-oriole-proto1_1.dtbo \
	google-dts-base/gs101-raven.dtbo \
	google-dts-base/gs101-raven-evt1_1.dtbo \
	google-dts-base/gs101-raven-evt-wingboard.dtbo \
	google-dts-base/gs101-raven-evt1.dtbo \
	google-dts-base/gs101-raven-proto1.dtbo \
	google-dts-base/gs101-raven-proto1_1.dtbo

slider_dtbs := \
	google-dts-base/gs101-a0.dtb \
	google-dts-base/gs101-b0.dtb

# Usage:
#   $(call DTBS_overlay_template, overlay, base_dtb, suffix)
#
# This is a generic template to create the following construct:
#   <overlay>-<suffix>-dtbs := <base_dtb>.dtb <overlay>.dtbo
define DTBS_overlay_template
	$(1:%.dtbo=%-$(strip $3)-dtbs) := $2 $1
endef

# Usage:
#   $(call DTB_object_template, CONFIG_, overlay, suffix)
#
# This is a generic template to create the following construct:
#   dtb-$(CONFIG_) += <overlay>-<suffix>.dtb
define DTB_object_template
	dtb-$(strip $1) += $(2:%.dtbo=%-$(strip $3).dtb)
endef

# Usage:
#   $(call multi_dtbs_overlay, list_of_base_dtbs, list_of_overlays, CONFIG_)
#
# For each (base_dtb,overlay) pair, this will create the following constructs:
#   <overlay>-<base_dtb>-dtbs := <base_dtb>.dtb <overlay>.dtbo
#   dtb-$(CONFIG_) += <overlay>-<base_dtb>.dtb
define multi_dtbs_overlay
	$(foreach base, $1, \
		$(foreach overlay, $2, \
			$(eval $(call DTBS_overlay_template, $(overlay), $(base), $(notdir $(base:%.dtb=%)))) \
			$(eval $(call DTB_object_template, $3, $(overlay), $(notdir $(base:%.dtb=%))))))
endef

# Create the -dtbs and dtb-y constructs. We can do this by looping through the
# base dtbs and apply each overlay to it. Kbuild will use these -dtbs
# constructs to determine which dtbs are the base dtbs and which are the
# overlays. For the base dtbs, kbuild will expose the symbols as needed for the
# overlays. Additionally, kbuild will statically apply the overlay to the base
# dtb which could be used to verify the overlay applies cleanly.
#$(call multi_dtbs_overlay, $(slider_dtbs), $(slider_overlays), $(CONFIG_SOC_ZUMA))
$(call multi_dtbs_overlay, $(slider_dtbs), $(slider_overlays), y)
