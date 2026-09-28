import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    ICON_TIMER,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_MOISTURE,
    DEVICE_CLASS_SPEED,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_WEIGHT,
    ENTITY_CATEGORY_DIAGNOSTIC,
    UNIT_CELSIUS,
    UNIT_MINUTE,
    UNIT_HOUR,
    UNIT_REVOLUTIONS_PER_MINUTE,
    UNIT_KILOGRAM,
    UNIT_PERCENT,
)

from . import CONF_ELECTROLUX_WASHING_MACHINE_MACS_ID, ElectroluxWashingMachineMacsComponent

CONF_REMAINING_TIME = "remaining_time"
CONF_START_DELAY_TIME = "start_delay_time"
CONF_SELECTED_PROGRAM_NUMBER = "selected_program_number"
CONF_TIME_MANAGER = "time_manager"
CONF_WASH_TEMPERATURE = "wash_temperature"
CONF_SPIN_SPEED = "spin_speed"
CONF_TARGET_DRUM_SPEED = "target_drum_speed"
CONF_CURRENT_DRUM_SPEED = "current_drum_speed"
CONF_CURRENT_WATER_TEMPERATURE = "current_water_temperature"
CONF_TOTAL_WORKING_HOURS = "total_working_hours"
CONF_TOTAL_CYCLES = "total_cycles"
CONF_SUB_PHASE = "sub_phase"
CONF_LAUNDRY_LOAD = "laundry_load"
CONF_PROGRAM_PROGRESS = "program_progress"
CONF_ELAPSED_TIME = "elapsed_time"
CONF_START_DELAY_PROGRESS = "start_delay_progress"

TYPES = [
    CONF_REMAINING_TIME,
    CONF_START_DELAY_TIME,
    CONF_SELECTED_PROGRAM_NUMBER,
    CONF_TIME_MANAGER,
    CONF_WASH_TEMPERATURE,
    CONF_SPIN_SPEED,
    CONF_TARGET_DRUM_SPEED,
    CONF_CURRENT_DRUM_SPEED,
    CONF_CURRENT_WATER_TEMPERATURE,
    CONF_TOTAL_WORKING_HOURS,
    CONF_TOTAL_CYCLES,
    CONF_SUB_PHASE,
    CONF_LAUNDRY_LOAD,
    CONF_PROGRAM_PROGRESS,
    CONF_ELAPSED_TIME,
    CONF_START_DELAY_PROGRESS,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ELECTROLUX_WASHING_MACHINE_MACS_ID): cv.use_id(ElectroluxWashingMachineMacsComponent),
        cv.Optional(CONF_REMAINING_TIME): sensor.sensor_schema(
            unit_of_measurement=UNIT_MINUTE,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_DURATION,
            icon=ICON_TIMER,
        ),
        cv.Optional(CONF_START_DELAY_TIME): sensor.sensor_schema(
            unit_of_measurement=UNIT_MINUTE,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_DURATION,
        ),
        cv.Optional(CONF_SELECTED_PROGRAM_NUMBER): sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_TIME_MANAGER): sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_WASH_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_TEMPERATURE,
        ),
        cv.Optional(CONF_SPIN_SPEED): sensor.sensor_schema(
            unit_of_measurement=UNIT_REVOLUTIONS_PER_MINUTE,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_SPEED,
        ),
        cv.Optional(CONF_TARGET_DRUM_SPEED): sensor.sensor_schema(
            unit_of_measurement=UNIT_REVOLUTIONS_PER_MINUTE,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_SPEED,
        ),
        cv.Optional(CONF_CURRENT_DRUM_SPEED): sensor.sensor_schema(
            unit_of_measurement=UNIT_REVOLUTIONS_PER_MINUTE,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_SPEED,
        ),
        cv.Optional(CONF_CURRENT_WATER_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_TEMPERATURE,
        ),
        cv.Optional(CONF_TOTAL_WORKING_HOURS): sensor.sensor_schema(
            unit_of_measurement=UNIT_HOUR,
            accuracy_decimals=0,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            device_class=DEVICE_CLASS_DURATION,
        ),
        cv.Optional(CONF_TOTAL_CYCLES): sensor.sensor_schema(
            accuracy_decimals=0,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            device_class=DEVICE_CLASS_DURATION,
        ),
        cv.Optional(CONF_SUB_PHASE): sensor.sensor_schema(
            accuracy_decimals=0,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_LAUNDRY_LOAD): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOGRAM,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_WEIGHT,
            icon="mdi:scale",
        ),
        # Program tracking: need time_id on the hub
        cv.Optional(CONF_PROGRAM_PROGRESS): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            icon="mdi:progress-clock",
        ),
        cv.Optional(CONF_ELAPSED_TIME): sensor.sensor_schema(
            unit_of_measurement=UNIT_MINUTE,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_DURATION,
            icon=ICON_TIMER,
        ),
        cv.Optional(CONF_START_DELAY_PROGRESS): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            icon="mdi:timer-sand",
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        sens = await sensor.new_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_sensor")(sens))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ELECTROLUX_WASHING_MACHINE_MACS_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)

