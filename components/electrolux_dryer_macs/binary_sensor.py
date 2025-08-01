import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_RUNNING,
    DEVICE_CLASS_HEAT
)

from . import CONF_ELECTROLUX_DRYER_MACS_ID, ElectroluxDryerMacsComponent

CONF_POWERED_ON = "powered_on"
CONF_RUNNING = "running"
CONF_DELICATE_MODE = "delicate_mode"
CONF_HEATING = "heating"

TYPES = [
    CONF_POWERED_ON,
    CONF_RUNNING,
    CONF_DELICATE_MODE,
    CONF_HEATING
]

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_ELECTROLUX_DRYER_MACS_ID): cv.use_id(ElectroluxDryerMacsComponent),
            cv.Optional(CONF_POWERED_ON): binary_sensor.binary_sensor_schema(device_class=DEVICE_CLASS_POWER),
            cv.Optional(CONF_RUNNING): binary_sensor.binary_sensor_schema(device_class=DEVICE_CLASS_RUNNING),
            cv.Optional(CONF_DELICATE_MODE): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_HEATING): binary_sensor.binary_sensor_schema(device_class=DEVICE_CLASS_HEAT),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        var = await binary_sensor.new_binary_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_binary_sensor")(var))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ELECTROLUX_DRYER_MACS_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)
