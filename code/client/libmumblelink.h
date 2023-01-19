
int mumble_link(const char* name);
int mumble_islinked(void);
void mumble_update_coordinates(float fPosition[3], float fFront[3], float fTop[3]);

/* new for mumble 1.2: also set camera position */
void mumble_update_coordinates2(float fAvatarPosition[3], float fAvatarFront[3], float fAvatarTop[3],
		float fCameraPosition[3], float fCameraFront[3], float fCameraTop[3]);

void mumble_set_description(const char* description);
void mumble_set_context(const unsigned char* context, size_t len);
void mumble_set_identity(const char* identity);

void mumble_unlink(void);
