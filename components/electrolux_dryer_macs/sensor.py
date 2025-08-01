import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    STATE_CLASS_MEASUREMENT,
    UNIT_MINUTE,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_MOISTURE
)

from . import CONF_ELECTROLUX_DRYER_MACS_ID, ElectroluxDryerMacsComponent

CONF_REMAINING_TIME = "remaining_time"
CONF_PROGRAM_DRYNESS_LEVEL = "program_dryness_level"
CONF_START_DELAY_TIME = "start_delay_time"
CONF_SELECTED_PROGRAM_NUMBER = "selected_program_number"

TYPES = [
    CONF_REMAINING_TIME,
    CONF_PROGRAM_DRYNESS_LEVEL,
    CONF_START_DELAY_TIME,
    CONF_SELECTED_PROGRAM_NUMBER
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ELECTROLUX_DRYER_MACS_ID): cv.use_id(ElectroluxDryerMacsComponent),
        cv.Optional(CONF_REMAINING_TIME): sensor.sensor_schema(
            unit_of_measurement=UNIT_MINUTE,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_DURATION,
        ),
        cv.Optional(CONF_START_DELAY_TIME): sensor.sensor_schema(
            unit_of_measurement=UNIT_MINUTE,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_DURATION,
        ),
        cv.Optional(CONF_PROGRAM_DRYNESS_LEVEL): sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            device_class=DEVICE_CLASS_MOISTURE,
        ),
        cv.Optional(CONF_SELECTED_PROGRAM_NUMBER): sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        sens = await sensor.new_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_sensor")(sens))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ELECTROLUX_DRYER_MACS_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)

