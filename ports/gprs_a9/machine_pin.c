/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>


#include "py/runtime.h"
#include "py/mphal.h"
#include "mphalport.h"
#include "modmachine.h"
#include "extmod/virtpin.h"

#include "api_hal_pm.h"
#include "api_hal_gpio.h"

typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    GPIO_PIN id;
} machine_pin_obj_t;

typedef struct _machine_pin_irq_obj_t {
    mp_obj_base_t base;
    GPIO_PIN id;
} machine_pin_irq_obj_t;

STATIC const machine_pin_obj_t machine_pin_obj[] = {
    {{&machine_pin_type}, GPIO_PIN0},
    {{&machine_pin_type}, GPIO_PIN1},
    {{&machine_pin_type}, GPIO_PIN2},
    {{&machine_pin_type}, GPIO_PIN3},
    {{&machine_pin_type}, GPIO_PIN4},
    {{&machine_pin_type}, GPIO_PIN5},
    {{&machine_pin_type}, GPIO_PIN6},
    {{&machine_pin_type}, GPIO_PIN7},
    {{&machine_pin_type}, GPIO_PIN8},
    {{&machine_pin_type}, GPIO_PIN9},
    {{&machine_pin_type}, GPIO_PIN10},
    {{&machine_pin_type}, GPIO_PIN11},
    {{&machine_pin_type}, GPIO_PIN12},
    {{&machine_pin_type}, GPIO_PIN13},
    {{&machine_pin_type}, GPIO_PIN14},
    {{&machine_pin_type}, GPIO_PIN15},
    {{&machine_pin_type}, GPIO_PIN16},
    {{&machine_pin_type}, GPIO_PIN17},
    {{&machine_pin_type}, GPIO_PIN18},
    {{&machine_pin_type}, GPIO_PIN19},
    {{&machine_pin_type}, GPIO_PIN20},
    {{&machine_pin_type}, GPIO_PIN21},
    {{&machine_pin_type}, GPIO_PIN22},
    {{&machine_pin_type}, GPIO_PIN23},
    {{&machine_pin_type}, GPIO_PIN24},
    {{&machine_pin_type}, GPIO_PIN25},
    {{&machine_pin_type}, GPIO_PIN26},
    {{&machine_pin_type}, GPIO_PIN27},
    {{&machine_pin_type}, GPIO_PIN28},
    {{&machine_pin_type}, GPIO_PIN29},
    {{&machine_pin_type}, GPIO_PIN30},
    {{&machine_pin_type}, GPIO_PIN31},
    {{&machine_pin_type}, GPIO_PIN32},
    {{&machine_pin_type}, GPIO_PIN33},
    {{&machine_pin_type}, GPIO_PIN34},
};

// forward declaration
// STATIC const machine_pin_irq_obj_t machine_pin_irq_object[];

// void machine_pins_init(void) {
//     static bool did_install = false;
//     if (!did_install) {
//         gpio_install_isr_service(0);
//         did_install = true;
//     }
//     memset(&MP_STATE_PORT(machine_pin_irq_handler[0]), 0, sizeof(MP_STATE_PORT(machine_pin_irq_handler)));
// }

// void machine_pins_deinit(void) {
//     for (int i = 0; i < MP_ARRAY_SIZE(machine_pin_obj); ++i) {
//         if (machine_pin_obj[i].id != (GPIO_PINt)-1) {
//             gpio_isr_handler_remove(machine_pin_obj[i].id);
//         }
//     }
// }

// STATIC void IRAM_ATTR machine_pin_isr_handler(void *arg) {
//     machine_pin_obj_t *self = arg;
//     mp_obj_t handler = MP_STATE_PORT(machine_pin_irq_handler)[self->id];
//     mp_sched_schedule(handler, MP_OBJ_FROM_PTR(self));
//     mp_hal_wake_main_task_from_isr();
// }

GPIO_PIN machine_pin_get_id(mp_obj_t pin_in) {
    if (mp_obj_get_type(pin_in) != &machine_pin_type) {
        mp_raise_ValueError("expecting a pin");
    }
    machine_pin_obj_t *self = pin_in;
    return self->id;
}

STATIC void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pin_obj_t *self = self_in;
    mp_printf(print, "Pin(%u)", self->id);
}

// pin.init(mode, pull=None, *, value)
STATIC mp_obj_t machine_pin_obj_init_helper(const machine_pin_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_mode, ARG_pull, ARG_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_pull, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    };
    GPIO_config_t gpioObj;

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // configure the pin for gpio
    if( (self->id >= 8) && self->id <=13 )
        PM_PowerEnable(POWER_TYPE_MMC,true);
    else if( (self->id >= 14) && self->id <=18 )
        PM_PowerEnable(POWER_TYPE_LCD,true);
    else if( (self->id >= 19) && self->id <=24 )
        PM_PowerEnable(POWER_TYPE_CAM,true);

    // set initial value (do this before configuring mode/pull)
    if (args[ARG_value].u_obj != MP_OBJ_NULL) {
        gpioObj.defaultLevel = mp_obj_is_true(args[ARG_value].u_obj);
    }

    // configure mode
    if (args[ARG_mode].u_obj != mp_const_none) {
        mp_int_t pin_io_mode = mp_obj_get_int(args[ARG_mode].u_obj);
        if(pin_io_mode & GPIO_MODE_INPUT)
            gpioObj.mode = GPIO_MODE_INPUT;
        else
            gpioObj.mode = GPIO_MODE_OUTPUT;
    }

    // configure pull
    if (args[ARG_pull].u_obj != mp_const_none) {
        gpioObj.defaultLevel = mp_obj_get_int(args[ARG_pull].u_obj);
    }
    gpioObj.pin = self->id;
    GPIO_Init(gpioObj);
    return mp_const_none;
}

// constructor(id, ...)
mp_obj_t mp_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // get the wanted pin object
    int wanted_pin = mp_obj_get_int(args[0]);
    const machine_pin_obj_t *self = NULL;
    if (0 <= wanted_pin && wanted_pin < MP_ARRAY_SIZE(machine_pin_obj)) {
        self = (machine_pin_obj_t*)&machine_pin_obj[wanted_pin];
    }
    if (self == NULL || self->base.type == NULL) {
        mp_raise_ValueError("invalid pin");
    }

    if (n_args > 1 || n_kw > 0) {
        // pin mode given, so configure this GPIO
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        machine_pin_obj_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return MP_OBJ_FROM_PTR(self);
}

// fast method for getting/setting pin value
STATIC mp_obj_t machine_pin_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    machine_pin_obj_t *self = self_in;
    GPIO_LEVEL level;

    if (n_args == 0) {
        // get pin
        GPIO_Get(self->id,&level);
        return MP_OBJ_NEW_SMALL_INT(level);
    } else {
        // set pin
        GPIO_Set(self->id,mp_obj_is_true(args[0]));
        return mp_const_none;
    }
}

// pin.init(mode, pull)
STATIC mp_obj_t machine_pin_obj_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_pin_obj_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_init_obj, 1, machine_pin_obj_init);

// pin.value([value])
STATIC mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args) {
    return machine_pin_call(args[0], n_args - 1, 0, args + 1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_value_obj, 1, 2, machine_pin_value);

// pin.irq(handler=None, trigger=IRQ_FALLING|IRQ_RISING)
// STATIC mp_obj_t machine_pin_irq(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
//     enum { ARG_handler, ARG_trigger, ARG_wake };
//     static const mp_arg_t allowed_args[] = {
//         { MP_QSTR_handler, MP_ARG_OBJ, {.u_obj = mp_const_none} },
//         { MP_QSTR_trigger, MP_ARG_INT, {.u_int = GPIO_PIN_INTR_POSEDGE | GPIO_PIN_INTR_NEGEDGE} },
//         { MP_QSTR_wake, MP_ARG_OBJ, {.u_obj = mp_const_none} },
//     };
//     machine_pin_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
//     mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
//     mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

//     if (n_args > 1 || kw_args->used != 0) {
//         // configure irq
//         mp_obj_t handler = args[ARG_handler].u_obj;
//         uint32_t trigger = args[ARG_trigger].u_int;
//         mp_obj_t wake_obj = args[ARG_wake].u_obj;

//         if ((trigger == GPIO_PIN_INTR_LOLEVEL || trigger == GPIO_PIN_INTR_HILEVEL) && wake_obj != mp_const_none) {
//             mp_int_t wake;
//             if (mp_obj_get_int_maybe(wake_obj, &wake)) {
//                 if (wake < 2 || wake > 7) {
//                     mp_raise_ValueError("bad wake value");
//                 }
//             } else {
//                 mp_raise_ValueError("bad wake value");
//             }

//             if (machine_rtc_config.wake_on_touch) { // not compatible
//                 mp_raise_ValueError("no resources");
//             }

//             if (!RTC_IS_VALID_EXT_PIN(self->id)) {
//                 mp_raise_ValueError("invalid pin for wake");
//             }

//             if (machine_rtc_config.ext0_pin == -1) {
//                 machine_rtc_config.ext0_pin = self->id;
//             } else if (machine_rtc_config.ext0_pin != self->id) {
//                 mp_raise_ValueError("no resources");
//             }

//             machine_rtc_config.ext0_level = trigger == GPIO_PIN_INTR_LOLEVEL ? 0 : 1;
//             machine_rtc_config.ext0_wake_types = wake;
//         } else {
//             if (machine_rtc_config.ext0_pin == self->id) {
//                 machine_rtc_config.ext0_pin = -1;
//             }

//             if (handler == mp_const_none) {
//                 handler = MP_OBJ_NULL;
//                 trigger = 0;
//             }
//             gpio_isr_handler_remove(self->id);
//             MP_STATE_PORT(machine_pin_irq_handler)[self->id] = handler;
//             gpio_set_intr_type(self->id, trigger);
//             gpio_isr_handler_add(self->id, machine_pin_isr_handler, (void*)self);
//         }
//     }

//     // return the irq object
//     return MP_OBJ_FROM_PTR(&machine_pin_irq_object[self->id]);
// }
// STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_irq_obj, 1, machine_pin_irq);

STATIC const mp_rom_map_elem_t machine_pin_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pin_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&machine_pin_value_obj) },
    // { MP_ROM_QSTR(MP_QSTR_irq), MP_ROM_PTR(&machine_pin_irq_obj) },

    // class constants
    { MP_ROM_QSTR(MP_QSTR_IN), MP_ROM_INT(GPIO_MODE_INPUT) },
    { MP_ROM_QSTR(MP_QSTR_OUT), MP_ROM_INT(GPIO_MODE_OUTPUT) },
    // { MP_ROM_QSTR(MP_QSTR_OPEN_DRAIN), MP_ROM_INT(GPIO_MODE_INPUT_OUTPUT_OD) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP), MP_ROM_INT(GPIO_LEVEL_HIGH) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN), MP_ROM_INT(GPIO_LEVEL_LOW) },
    // { MP_ROM_QSTR(MP_QSTR_IRQ_RISING), MP_ROM_INT(GPIO_PIN_INTR_POSEDGE) },
    // { MP_ROM_QSTR(MP_QSTR_IRQ_FALLING), MP_ROM_INT(GPIO_PIN_INTR_NEGEDGE) },
    // { MP_ROM_QSTR(MP_QSTR_WAKE_LOW), MP_ROM_INT(GPIO_PIN_INTR_LOLEVEL) },
    // { MP_ROM_QSTR(MP_QSTR_WAKE_HIGH), MP_ROM_INT(GPIO_PIN_INTR_HILEVEL) },
};

STATIC mp_uint_t pin_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    (void)errcode;
    machine_pin_obj_t *self = self_in;
    GPIO_LEVEL level;

    switch (request) {
        case MP_PIN_READ: {
            GPIO_Get(self->id,&level);
            return level;
        }
        case MP_PIN_WRITE: {
            GPIO_Set(self->id,arg);
            return 0;
        }
    }
    return -1;
}

STATIC MP_DEFINE_CONST_DICT(machine_pin_locals_dict, machine_pin_locals_dict_table);

STATIC const mp_pin_p_t pin_pin_p = {
  .ioctl = pin_ioctl,
};

const mp_obj_type_t machine_pin_type = {
    { &mp_type_type },
    .name = MP_QSTR_Pin,
    .print = machine_pin_print,
    .make_new = mp_pin_make_new,
    .call = machine_pin_call,
    .protocol = &pin_pin_p,
    .locals_dict = (mp_obj_t)&machine_pin_locals_dict,
};

/******************************************************************************/
// Pin IRQ object

STATIC const mp_obj_type_t machine_pin_irq_type;

STATIC const machine_pin_irq_obj_t machine_pin_irq_object[] = {
    {{&machine_pin_irq_type}, GPIO_PIN0},
    {{&machine_pin_irq_type}, GPIO_PIN1},
    {{&machine_pin_irq_type}, GPIO_PIN2},
    {{&machine_pin_irq_type}, GPIO_PIN3},
    {{&machine_pin_irq_type}, GPIO_PIN4},
    {{&machine_pin_irq_type}, GPIO_PIN5},
    {{&machine_pin_irq_type}, GPIO_PIN6},
    {{&machine_pin_irq_type}, GPIO_PIN7},
};

// STATIC mp_obj_t machine_pin_irq_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
//     machine_pin_irq_obj_t *self = self_in;
//     mp_arg_check_num(n_args, n_kw, 0, 0, false);
//     machine_pin_isr_handler((void*)&machine_pin_obj[self->id]);
//     return mp_const_none;
// }

// STATIC mp_obj_t machine_pin_irq_trigger(size_t n_args, const mp_obj_t *args) {
//     machine_pin_irq_obj_t *self = args[0];
//     uint32_t orig_trig = GPIO.pin[self->id].int_type;
//     if (n_args == 2) {
//         // set trigger
//         gpio_set_intr_type(self->id, mp_obj_get_int(args[1]));
//     }
//     // return original trigger value
//     return MP_OBJ_NEW_SMALL_INT(orig_trig);
// }
// STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_irq_trigger_obj, 1, 2, machine_pin_irq_trigger);

// STATIC const mp_rom_map_elem_t machine_pin_irq_locals_dict_table[] = {
//     { MP_ROM_QSTR(MP_QSTR_trigger), MP_ROM_PTR(&machine_pin_irq_trigger_obj) },
// };
// STATIC MP_DEFINE_CONST_DICT(machine_pin_irq_locals_dict, machine_pin_irq_locals_dict_table);

// STATIC const mp_obj_type_t machine_pin_irq_type = {
//     { &mp_type_type },
//     .name = MP_QSTR_IRQ,
//     .call = machine_pin_irq_call,
//     .locals_dict = (mp_obj_dict_t*)&machine_pin_irq_locals_dict,
// };
