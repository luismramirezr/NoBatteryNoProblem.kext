/*
 * Copyright (c) 2012 Adam Strzelecki
 * Copyright (c) 2015 Bernardo Alecrim
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Description:
 *   Disables upon load and re-enables upon unload TurboBoost and BD Prochot
 *   Intel Core CPU features using MSR register MSR_IA32_MISC_ENABLE (0x1a0)
 *	 and ?? (0x1FC).
 */

#include <Kernel/mach/mach_types.h>
#include <Kernel/libkern/libkern.h>
#include <Kernel/i386/proc_reg.h>

extern void mp_rendezvous_no_intrs(
		void (*action_func)(void *),
		void *arg);

//Using the static data from unclewebb for now.
static void disable_prochot(__unused void *param_not_used)
{
	wrmsr64(0x1FC, rdmsr64(0x1FC) & 0xFFFFFFFE);
}

static void enable_prochot(__unused void *param_not_used)
{
	wrmsr64(0x1FC, rdmsr64(0x1FC) | 0x1);
}

static kern_return_t start(kmod_info_t *ki, void *d)
{
	uint64_t prev = rdmsr64(MSR_IA32_MISC_ENABLE);
	mp_rendezvous_no_intrs(disable_prochot, NULL);
	printf("Disabled BD PROCHOT: %llx -> %llx\n", prev, rdmsr64(MSR_IA32_MISC_ENABLE));
	return KERN_SUCCESS;
}

static kern_return_t stop(kmod_info_t *ki, void *d)
{
	uint64_t prev = rdmsr64(MSR_IA32_MISC_ENABLE);
	mp_rendezvous_no_intrs(enable_prochot, NULL);
	printf("Re-enabled BD PROCHOT: %llx -> %llx\n", prev, rdmsr64(MSR_IA32_MISC_ENABLE));
	return KERN_SUCCESS;
}

extern kern_return_t _start(kmod_info_t *ki, void *data);
extern kern_return_t _stop(kmod_info_t *ki, void *data);

KMOD_EXPLICIT_DECL(me.conductive.bernardo.NoBatteryNoProblem, "0.0.1", _start, _stop)
__private_extern__ kmod_start_func_t *_realmain = start;
__private_extern__ kmod_stop_func_t *_antimain = stop;
__private_extern__ int _kext_apple_cc = __APPLE_CC__;
