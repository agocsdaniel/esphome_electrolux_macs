import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_RUNNING,
    DEVICE_CLASS_DOOR,
    DEVICE_CLASS_MOISTURE,
    DEVICE_CLASS_MOVING,
)

from . import CONF_ELECTROLUX_WASHING_MACHINE_MACS_ID, ElectroluxWashingMachineMacsComponent

CONF_POWERED_ON = "powered_on"
CONF_RUNNING = "running"
CONF_DOOR_LOCKED = "door_locked"
CONF_PRE_WASH_ENABLED = "pre_wash_enabled"
CONF_STAIN_WASH = "stain_wash"
CONF_EXTRA_RINSE = "extra_rinse"
CONF_SOFT_PLUS = "soft_plus"
CONF_EASY_IRON = "easy_iron"
CONF_ANTI_CREASE = "anti_crease"
CONF_WASHING_ENABLED = "washing_enabled"
CONF_DRAIN_PUMP = "drain_pump"
CONF_WATER_IN_DRUM = "water_in_drum"
CONF_DRUM_TURNING = "drum_turning"
CONF_DOOR_OPEN = "door_open"

TYPES = [
    CONF_POWERED_ON,
    CONF_RUNNING,
    CONF_DOOR_LOCKED,
    CONF_PRE_WASH_ENABLED,
    CONF_STAIN_WASH,
    CONF_EXTRA_RINSE,
    CONF_SOFT_PLUS,
    CONF_EASY_IRON,
    CONF_ANTI_CREASE,
    CONF_WASHING_ENABLED,
    CONF_DRAIN_PUMP,
    CONF_WATER_IN_DRUM,
    CONF_DRUM_TURNING,
    CONF_DOOR_OPEN,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ELECTROLUX_WASHING_MACHINE_MACS_ID): cv.use_id(ElectroluxWashingMachineMacsComponent),
        cv.Optional(CONF_POWERED_ON): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_POWER
        ),
        cv.Optional(CONF_RUNNING): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_RUNNING,
        ),
        cv.Optional(CONF_DOOR_LOCKED): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_DOOR,
        ),
        cv.Optional(CONF_PRE_WASH_ENABLED): binary_sensor.binary_sensor_schema(
        ),
        cv.Optional(CONF_STAIN_WASH): binary_sensor.binary_sensor_schema(
        ),
        cv.Optional(CONF_EXTRA_RINSE): binary_sensor.binary_sensor_schema(
        ),
        cv.Optional(CONF_SOFT_PLUS): binary_sensor.binary_sensor_schema(
        ),
        cv.Optional(CONF_EASY_IRON): binary_sensor.binary_sensor_schema(
        ),
        cv.Optional(CONF_ANTI_CREASE): binary_sensor.binary_sensor_schema(
        ),
        cv.Optional(CONF_WASHING_ENABLED): binary_sensor.binary_sensor_schema(
        ),
        cv.Optional(CONF_DRAIN_PUMP): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_RUNNING,
        ),
        cv.Optional(CONF_WATER_IN_DRUM): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_MOISTURE,
        ),
        cv.Optional(CONF_DRUM_TURNING): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_MOVING,
        ),
        # EW8W261B only
        cv.Optional(CONF_DOOR_OPEN): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_DOOR,
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        var = await binary_sensor.new_binary_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_binary_sensor")(var))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ELECTROLUX_WASHING_MACHINE_MACS_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)
