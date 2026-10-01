#pragma once

#include <graphics/graphics.h>

/* Caller holds the graphics context. The shared resource remains callback-owned. */
inline bool CopyBrowserSharedTexture(gs_texture_t *&owned, gs_texture_t *shared, bool keyed_mutex)
{
	if (!shared)
		return false;

	/* Modern CEF textures have no mutex; only synchronize an actual keyed texture. */
	if (keyed_mutex && gs_texture_acquire_sync(shared, 1, 0) != 0)
		return false;

	const uint32_t cx = gs_texture_get_width(shared);
	const uint32_t cy = gs_texture_get_height(shared);
	const gs_color_format format = gs_texture_get_color_format(shared);
	gs_texture_t *target = owned;

	if (!cx || !cy || format == GS_UNKNOWN) {
		if (keyed_mutex)
			gs_texture_release_sync(shared, 0);
		return false;
	}

	if (!target || gs_texture_get_width(target) != cx || gs_texture_get_height(target) != cy ||
	    gs_texture_get_color_format(target) != format) {
		target = gs_texture_create(cx, cy, format, 1, nullptr, 0);
		if (!target) {
			if (keyed_mutex)
				gs_texture_release_sync(shared, 0);
			return false;
		}
	}

	gs_copy_texture(target, shared);
	/* Submit while CEF still owns the pooled resource, before handing back a mutex. */
	gs_flush();
	if (keyed_mutex)
		gs_texture_release_sync(shared, 0);

	if (target != owned) {
		gs_texture_destroy(owned);
		owned = target;
	}
	return true;
}
