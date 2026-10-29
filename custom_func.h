/* Project/API reading notes: User entry declarations shared with the selected main/startup
 * adapter.
 * Implement application initialization in hardware_init() and recurring foreground work in
 * loop(); peripheral ISR declarations remain with their owning driver.
 */
#ifndef CUSTOM_FUNC_H
#define CUSTOM_FUNC_H

/* User-owned initialization and one foreground iteration. */
void hardware_init(void);
void loop(void);

#endif
