import esphome.codegen as cg
from esphome.components import time as time_, uart
import esphome.config_validation as cv
from ..electrolux_macs import CONFIG_SCHEMA_BASE, CONF_ID, CONF_UART_ID, CONF_RECEIVE_TIMEOUT, CONF_VERIFY_CHECKSUM

CODEOWNERS = ["@agocsdaniel"]

MULTI_CONF = True

AUTO_LOAD = ["electrolux_macs", "binary_sensor", "sensor", "text_sensor"]

CONF_ELECTROLUX_WASHING_MACHINE_MACS_ID = "electrolux_washing_machine_macs_id"

CONF_MOTOR_DRUM_RATIO = "motor_drum_ratio"
CONF_MODEL = "model"
CONF_TIME_ID = "time_id"
MODELS = {"EWX14": 0, "EW8W261B": 1}

electrolux_washing_machine_macs_ns = cg.esphome_ns.namespace("electrolux_washing_machine_macs")
ElectroluxWashingMachineMacsComponent = electrolux_washing_machine_macs_ns.class_(
    "ElectroluxWashingMachineMacsComponent", cg.Component, uart.UARTDevice
)

CONFIG_SCHEMA = CONFIG_SCHEMA_BASE.extend({
    cv.GenerateID(): cv.declare_id(ElectroluxWashingMachineMacsComponent),
    cv.Optional(CONF_MOTOR_DRUM_RATIO, default=12.2): cv.float_range(0, 100),
    # Selects the model-specific frame layout (start delay byte, door lock bit), see EWX FINDINGS.md
    cv.Optional(CONF_MODEL, default="EWX14"): cv.one_of(*MODELS, upper=True),
    # Clock for program tracking (progress, elapsed time, start, estimated end)
    cv.Optional(CONF_TIME_ID): cv.use_id(time_.RealTimeClock),
})

async def to_code(config):
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_component)
    cg.add(var.set_receive_timeout(config[CONF_RECEIVE_TIMEOUT].total_milliseconds))
    cg.add(var.set_verify_checksum(config[CONF_VERIFY_CHECKSUM]))
    cg.add(var.set_motor_drum_ratio(config[CONF_MOTOR_DRUM_RATIO]))
    cg.add(var.set_ew8w(MODELS[config[CONF_MODEL]] == 1))
    if CONF_TIME_ID in config:
        cg.add(var.set_time(await cg.get_variable(config[CONF_TIME_ID])))
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

