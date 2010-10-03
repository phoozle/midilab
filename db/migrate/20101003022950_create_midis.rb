class CreateMidis < ActiveRecord::Migration
  def self.up
    create_table :midis do |t|
      t.binary :data
      t.string :name

      t.timestamps
    end
  end

  def self.down
    drop_table :midis
  end
end
